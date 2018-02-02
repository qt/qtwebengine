load(qt_parts)
# Upstream PDFium has not been ported to various platforms yet.
requires(!qnx:!uikit:!winphone:!winrt:!win32-g++:!integrity)
