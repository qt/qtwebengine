#!/bin/env python3
# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# See main() below, or run with -h, for usage.

import base64
import difflib
import sys
import argparse
import json
import os
import re
import logging
import shlex
import urllib.request

logger = logging.getLogger(__name__)
logging.basicConfig()

def diff_one_file(file_info, diff_method, context_lines):
    file_from = file_info['from']
    file_to = file_info['to']
    lines_from = file_info['qt']
    lines_to = file_info['external']

    if diff_method == 'unified':
        result = difflib.unified_diff(lines_from, lines_to, file_from, file_to, n=context_lines)
    elif diff_method == 'ndiff':
        result = difflib.ndiff(lines_from, lines_to)
    elif diff_method == 'context':
        result = difflib.context_diff(lines_from, lines_to, file_from, file_to, n=context_lines)

    result = list(result)
    if result and diff_method == 'ndiff':
        if file_info['from'] == file_info['to']:
            chunk_label = f'*** {file_info["from"]}\n'
        else:
            chunk_label = f'*** {file_info["from"]} -> {file_info["to"]}\n'
        return [chunk_label] + result + ['\n']
    if result:
        return result + ['\n']
    else:
        return []


def match_to_existing_file(chunk_map, filename_from, filename_to):
    found = None
    for fn, val in chunk_map.items():
        if fn.endswith(filename_from):
            assert not found, f'{filename_from} matched to two different files ({found}, {fn})'
            assert val['to'].endswith(filename_to), f'{filename_from} matched to {fn} but {filename_to} doesn\'t match with {val["to"]}'
            found = fn
    if found:
        result = (found, chunk_map[found]['to'])
        logger.debug(f'match_to_existing_file: matched ({filename_from}, {filename_to}) -> {result}')
        return result
    else:
        logger.debug(f'match_to_existing_file: no match for ({filename_from}, {filename_to})')
        return (filename_from, filename_to)


def add_chunks(chunk_map, lines, label, match_to_existing_files):
    lines = lines.splitlines(True)

    i = 0
    while i < len(lines) and not lines[i] == '---\n':
        i += 1
    assert i != len(lines), lines
    while i < len(lines) and not lines[i].startswith('diff'):
        i += 1
    if i == len(lines):
        return

    while True:
        logger.debug(f'add_chunks: line={lines[i].strip()}')
        # If a filename contains a space it will be surrounded with quotes; shlex.split
        # roughly matches the same logic.
        diff_args = shlex.split(lines[i])
        # Remove the leading a/ and b/ so patch prefix matching works
        filename_from = diff_args[2][2:]
        filename_to = diff_args[2][2:]
        if match_to_existing_files:
            filename_from, filename_to = match_to_existing_file(chunk_map, filename_from, filename_to)

        i += 5 # skip some context
        range_start = i
        assert range_start < len(lines)
        chunk = []
        while i < len(lines) and not lines[i].startswith('diff'):
            # Replace @@ <line numbers> @@ to avoid extra noise
            if lines[i].startswith('@@'):
                chunk.append('@@ CHUNK START @@')
            else:
                chunk.append(lines[i])
            i += 1

        chunk_map.setdefault(filename_from, dict())
        chunk_map[filename_from][label] = chunk
        chunk_map[filename_from]['from'] = filename_from
        if 'to' in chunk_map[filename_from]:
            assert chunk_map[filename_from]['to'] == filename_to
        else:
            chunk_map[filename_from]['to'] = filename_to
        if i == len(lines):
            return


def get_chunks(lines1, lines2):
    chunk_map = dict()
    add_chunks(chunk_map, lines1, 'qt', False)
    add_chunks(chunk_map, lines2, 'external', True)
    return chunk_map


def assemble_diff(chunk_map, diff_method, context_lines, report_unchanged):
    r = []
    headers = []
    for fn, value in chunk_map.items():
        if 'qt' in value and 'external' in value:
            if diff := diff_one_file(value, diff_method, context_lines):
                r += diff
            elif report_unchanged:
                headers.append(f'unchanged: {fn}\n')
        else:
            label = 'qt' if 'qt' in value else 'external'
            headers.append(f'only in {label} patch: {fn}\n')
    if headers:
        headers = ['~~~~~\n'] + headers + ['~~~~~\n\n']
    return headers + r

def download(url):
    logger.debug(f"download: {url}")
    return urllib.request.urlopen(url).read()


def get_latest_patchset_revision(query_url):
    logger.debug(f"get_latest_patchset_revision: {query_url}")

    contents = download(query_url)
    logger.debug(f"get_latest_patchset_revision: download -> {contents}")

    contents = contents.split(b'\n')[-2]
    logger.debug(f"get_latest_patchset_revision: last line -> {contents}")

    contents = json.loads(contents)
    logger.debug(f"get_latest_patchset_revision: to json -> {contents}")

    revision_num = contents['current_revision_number']
    logger.debug(f"get_latest_patchset_revision: revision number -> {revision_num}")

    return revision_num


def resolve_ref(ref, api_url_base):
    logger.debug(f"resolve_ref: ref={ref} api_url_base={api_url_base}")

    # Common formats:
    # https://codereview.qt-project.org/c/qt/qtwebengine-chromium/+/734233
    # https://codereview.qt-project.org/c/qt/qtwebengine-chromium/+/734233/2
    # 734233/2
    # 734233

    # Pulls last "<num>" or "<num>/" or "<num>/<num>".
    if m := re.fullmatch(r'.*?(\d+)/?(\d+)?', ref):
        review_num = m.group(1)
        revision_num = m.group(2) or get_latest_patchset_revision(f'{api_url_base}{review_num}')
    else:
        raise ValueError(f'Could not resolve ref: "{ref}"')

    return f'{api_url_base}{review_num}/revisions/{revision_num}/patch?download'


def get_patch_contents(ref, api_url_base):
    logger.debug(f"get_patch_contents: ref={ref} api_url_base={api_url_base}")
    url = resolve_ref(ref, api_url_base)
    base64_content = download(url)
    decoded = base64.b64decode(base64_content)
    try:
        return decoded.decode('utf-8', errors='strict')
    except UnicodeDecodeError as e:
        logger.warning(f'get_patch_contents: decoding to utf-8 had errors, probably because of a binary file in the diff. Some diffs may be inaccurate')
        return decoded.decode('utf-8', errors='ignore')


def get_external_patch_url(contents, fallback_to_first_url):
    lines = contents.split('\n')
    logger.debug(f"get_external_patch_url: {len(lines)} lines")

    first_url = None
    for l in lines:
        logger.debug(f"get_external_patch_url: line: {l}")
        if m := re.match(r'Reviewed-[oO]n: +(http.*)', l):
            if 'qt-project' in m.group(1):
                continue
            logger.debug(f"get_external_patch_url: line match: '{l}'")
            return m.group(1)
        elif m := re.match(r'.*?(https?://[^ :]+)', l):
            if 'qt-project' in m.group(1):
                continue
            first_url = m.group(1)
        elif re.fullmatch('---', l):
            break

    if fallback_to_first_url and first_url:
        return first_url

    raise ValueError(f'Could not find external URL in patch contents')


def get_external_api_url_base(url):
    # Example transformation:
    #   Input: "https://chromium-review.googlesource.com/c/chromium/src/+/7606759
    #   Output: "https://chromium-review.googlesource.com/changes/chromium%2Fsrc~"
    logger.debug(f"get_external_api_url_base: url={url}")
    match = re.fullmatch(r'(.*?)/c/(.*?)/\+/.*', url)
    escaped_inner_path = match.group(2).replace('/', '%2F')
    return f'{match.group(1)}/changes/{escaped_inner_path}~'


def main(args):
    qt_api_url_base = 'https://codereview.qt-project.org/changes/qt%2Fqtwebengine-chromium~'
    qt_patch_contents = get_patch_contents(args.qt_patch_ref, qt_api_url_base)

    external_patch_url = args.external_patch_url
    if external_patch_url is None:
        external_patch_url = get_external_patch_url(qt_patch_contents, args.fallback_to_first_url)

    external_api_url_base = get_external_api_url_base(external_patch_url)
    external_patch_contents = get_patch_contents(external_patch_url, external_api_url_base)

    chunk_map = get_chunks(qt_patch_contents, external_patch_contents)
    diff = assemble_diff(chunk_map, args.diff_method, args.context_lines, args.report_unchanged)

    sys.stdout.writelines(diff)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=
f"""Takes a review URL or ID (for example: "https://codereview.qt-project.org/c/qt/qtwebengine-chromium/+/737091/1",
"737091/1", or just "737091"). Finds the corresponding Chromium/external patch, compares the two, and prints a diff.
The external URL can be supplied directly if needed.

Basic usage: {sys.argv[0]} 737091
""")
    parser.add_argument('qt_patch_ref', help='Can be full Gerrit URL, <rvw-num>/<revision>, or just <rvw-num>')
    parser.add_argument('external_patch_url', nargs='?', help='Full Gerrit URL. If not provided, will be inferred from Qt patch contents')
    parser.add_argument('-v', '--verbose', action='store_true')
    parser.add_argument('-f', '--fallback-to-first-url', action='store_true', help='If no "Reviewed-on:" line is found in the Qt patch, fallback to using the first URL in the commit message')
    parser.add_argument('-d', '--diff-method', help='Diff method to use, one of [unified, ndiff, context]', default='unified')
    parser.add_argument('-C', '--context-lines', type=int, help='Num lines to show for context diff', default=3)
    parser.add_argument('-u', '--report-unchanged', action='store_true', help='Report files with no diff')

    args = parser.parse_args()
    if args.verbose:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    logger.debug(f'Args: {sys.argv}')
    main(args)
