// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "base/strings/string_util.h"

#include "qtwebengine/userscript/user_script_data.h"
#include "user_script.h"
#include "type_conversion.h"

namespace {

// Helper function to parse Greasemonkey headers
bool GetDeclarationValue(const base::StringPiece& line,
                         const base::StringPiece& prefix,
                         std::string* value) {
    base::StringPiece::size_type index = line.find(prefix);
    if (index == base::StringPiece::npos)
        return false;

    std::string temp(line.data() + index + prefix.length(),
                     line.length() - index - prefix.length());

    if (temp.empty() || !base::IsWhitespace(temp[0]))
        return false;

    base::TrimWhitespaceASCII(temp, base::TRIM_ALL, value);
        return true;
}

}  // namespace

namespace QtWebEngineCore {

ASSERT_ENUMS_MATCH(UserScript::AfterLoad, UserScriptData::AfterLoad)
ASSERT_ENUMS_MATCH(UserScript::DocumentLoadFinished, UserScriptData::DocumentLoadFinished)
ASSERT_ENUMS_MATCH(UserScript::DocumentElementCreation, UserScriptData::DocumentElementCreation)

UserScript::UserScript()
    : QSharedData()
{
    static uint64_t idCount = 0;
    m_scriptData.scriptId = idCount++;
}

UserScript::UserScript(const UserScript &other) = default;

UserScript::~UserScript() = default;

UserScript &UserScript::operator=(const UserScript &other)
{
    m_scriptData = other.m_scriptData;
    m_name = other.m_name;
    m_url = other.m_url;
    return *this;
}

QString UserScript::name() const
{
    return m_name;
}

void UserScript::setName(const QString &name)
{
    m_name = name;
    m_scriptData.url = GURL(QStringLiteral("userScript:%1").arg(name).toStdString());
}

QString UserScript::sourceCode() const
{
    return toQt(m_scriptData.source);
}

void UserScript::setSourceCode(const QString &source)
{
    m_scriptData.source = source.toStdString();
    parseMetadataHeader();
}

QUrl UserScript::sourceUrl() const
{
    return m_url;
}

void UserScript::setSourceUrl(const QUrl &url)
{
    m_url = url;
}

UserScript::InjectionPoint UserScript::injectionPoint() const
{
    return static_cast<UserScript::InjectionPoint>(m_scriptData.injectionPoint);
}

void UserScript::setInjectionPoint(UserScript::InjectionPoint p)
{
    m_scriptData.injectionPoint = p;
}

quint32 UserScript::worldId() const
{
    return m_scriptData.worldId;
}

void UserScript::setWorldId(quint32 id)
{
    m_scriptData.worldId = id;
}

bool UserScript::runsOnSubFrames() const
{
    return m_scriptData.injectForSubframes;
}

void UserScript::setRunsOnSubFrames(bool on)
{
    m_scriptData.injectForSubframes = on;
}

const UserScriptData &UserScript::data() const
{
    return m_scriptData;
}

bool UserScript::operator==(const UserScript &other) const
{
    return worldId() == other.worldId() && runsOnSubFrames() == other.runsOnSubFrames()
            && injectionPoint() == other.injectionPoint() && name() == other.name()
            && sourceCode() == other.sourceCode() && sourceUrl() == other.sourceUrl();
}

void UserScript::parseMetadataHeader()
{
    // Clear previous values
    m_scriptData.globs.clear();
    m_scriptData.excludeGlobs.clear();
    m_scriptData.urlPatterns.clear();

    // Logic taken from Chromium (extensions/browser/user_script_loader.cc)
    // http://wiki.greasespot.net/Metadata_block
    const std::string &script_text = m_scriptData.source;
    base::StringPiece line;
    size_t line_start = 0;
    size_t line_end = line_start;
    bool in_metadata = false;

    static const base::StringPiece kUserScriptBegin("// ==UserScript==");
    static const base::StringPiece kUserScriptEnd("// ==/UserScript==");
    static const base::StringPiece kNameDeclaration("// @name");
    static const base::StringPiece kIncludeDeclaration("// @include");
    static const base::StringPiece kExcludeDeclaration("// @exclude");
    static const base::StringPiece kMatchDeclaration("// @match");
    static const base::StringPiece kRunAtDeclaration("// @run-at");
    static const base::StringPiece kRunAtDocumentStartValue("document-start");
    static const base::StringPiece kRunAtDocumentEndValue("document-end");
    static const base::StringPiece kRunAtDocumentIdleValue("document-idle");
    // FIXME: Scripts don't run in subframes by default. If we would like to
    // support @noframes rule, we have to change the current default behavior.
    // static const base::StringPiece kNoFramesDeclaration("// @noframes");

    while (line_start < script_text.length()) {
        line_end = script_text.find('\n', line_start);

        // Handle the case where there is no trailing newline in the file.
        if (line_end == std::string::npos)
            line_end = script_text.length() - 1;

        line = base::StringPiece(script_text.data() + line_start, line_end - line_start);

        if (!in_metadata) {
            if (base::StartsWith(line, kUserScriptBegin))
                in_metadata = true;
        } else {
            if (base::StartsWith(line, kUserScriptEnd))
                break;

            std::string value;
            if (GetDeclarationValue(line, kNameDeclaration, &value)) {
                setName(toQt(value));
            } else if (GetDeclarationValue(line, kIncludeDeclaration, &value)) {
                if (value.front() != '/' || value.back() != '/') {
                  // The greasemonkey spec only allows for wildcards (*), so
                  // escape the additional things which MatchPattern allows.
                  base::ReplaceSubstringsAfterOffset(&value, 0, "\\", "\\\\");
                  base::ReplaceSubstringsAfterOffset(&value, 0, "?", "\\?");
                }
                m_scriptData.globs.push_back(value);
            } else if (GetDeclarationValue(line, kExcludeDeclaration, &value)) {
                if (value.front() != '/' || value.back() != '/') {
                  // The greasemonkey spec only allows for wildcards (*), so
                  // escape the additional things which MatchPattern allows.
                  base::ReplaceSubstringsAfterOffset(&value, 0, "\\", "\\\\");
                  base::ReplaceSubstringsAfterOffset(&value, 0, "?", "\\?");
                }
                m_scriptData.excludeGlobs.push_back(value);
            } else if (GetDeclarationValue(line, kMatchDeclaration, &value)) {
                m_scriptData.urlPatterns.push_back(value);
            } else if (GetDeclarationValue(line, kRunAtDeclaration, &value)) {
                if (value == kRunAtDocumentStartValue)
                    m_scriptData.injectionPoint = DocumentElementCreation;
                else if (value == kRunAtDocumentEndValue)
                    m_scriptData.injectionPoint = DocumentLoadFinished;
                else if (value == kRunAtDocumentIdleValue)
                    m_scriptData.injectionPoint = AfterLoad;
            }
        }

        line_start = line_end + 1;
    }

    // If no patterns were specified, default to @include *. This is what
    // Greasemonkey does.
    if (m_scriptData.globs.empty() && m_scriptData.urlPatterns.empty())
        m_scriptData.globs.push_back("*");
}

} // namespace QtWebEngineCore
