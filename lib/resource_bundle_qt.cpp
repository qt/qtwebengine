/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/data_pack.h"

#include <QFile>

namespace ui {

// ********************* data_pack.cc *********************
// This is duplicated code originating from data_pack.cc.
// It should instead be moved to a header file and be included
// in both places.

static const uint32 kFileFormatVersion = 4;
static const size_t kHeaderLength = 2 * sizeof(uint32) + sizeof(uint8);

#pragma pack(push,2)
struct DataPackEntry {
  uint16 resource_id;
  uint32 file_offset;

  static int CompareById(const void* void_key, const void* void_entry) {
    uint16 key = *reinterpret_cast<const uint16*>(void_key);
    const DataPackEntry* entry =
        reinterpret_cast<const DataPackEntry*>(void_entry);
    if (key < entry->resource_id) {
      return -1;
    } else if (key > entry->resource_id) {
      return 1;
    } else {
      return 0;
    }
  }
};
#pragma pack(pop)
// ******************* data_pack.cc end *******************

class UI_EXPORT DataPackQt : public DataPack {
    public:
        DataPackQt(ui::ScaleFactor scale_factor)
            : DataPack(scale_factor)
            , m_data(NULL)
            , m_resourceCount(0) { }

        virtual ~DataPackQt() { }

        bool LoadFromByteArray(const QByteArray& data)
        {
            m_data = data;

            if (kHeaderLength > m_data.size())
                return false;

            const uint32* ptr = reinterpret_cast<const uint32*>(m_data.data());
            uint32 version = ptr[0];
            if (version != kFileFormatVersion) {
                LOG(ERROR) << "Bad data pack version: got " << version << ", expected " << kFileFormatVersion;
                return false;
            }

            m_resourceCount = ptr[1];
            return true;
        }

        virtual bool HasResource(uint16 resource_id) const OVERRIDE
        {
            return !!bsearch(&resource_id, m_data.data() + kHeaderLength, m_resourceCount, sizeof(DataPackEntry), DataPackEntry::CompareById);
        }

        virtual bool GetStringPiece(uint16 resource_id, base::StringPiece* data) const OVERRIDE
        {
            #if defined(__BYTE_ORDER) // Linux check
                COMPILE_ASSERT(__BYTE_ORDER == __LITTLE_ENDIAN, datapack_assumes_little_endian);
            #elif defined(__BIG_ENDIAN__) // Mac check
                #error DataPack assumes little endian
            #endif

            const DataPackEntry* target = reinterpret_cast<const DataPackEntry*>(bsearch(&resource_id, m_data.data() + kHeaderLength, m_resourceCount, sizeof(DataPackEntry), DataPackEntry::CompareById));
            if (!target)
                return false;

            const DataPackEntry* next_entry = target + 1;
            size_t length = next_entry->file_offset - target->file_offset;

            data->set(m_data.data() + target->file_offset, length);
            return true;
        }

    private:
        QByteArray m_data;
        size_t m_resourceCount;
        DISALLOW_COPY_AND_ASSIGN(DataPackQt);
};


void ResourceBundle::LoadCommonResources()
{
    QFile pak_file(":/data/resources.pak");
    if (!pak_file.open(QIODevice::ReadOnly))
        return;

    scoped_ptr<DataPackQt> data_pack(new DataPackQt(SCALE_FACTOR_100P));
    if (data_pack->LoadFromByteArray(pak_file.readAll()))
        AddDataPack(data_pack.release());
}

base::FilePath ResourceBundle::GetLocaleFilePath(const std::string& /*app_locale*/, bool /*test_file_exists*/)
{
    return base::FilePath();
}

gfx::Image& ResourceBundle::GetNativeImageNamed(int resource_id, ImageRTL rtl)
{
    LOG(WARNING) << "Unable to load image with id " << resource_id;
    NOTREACHED();  // Want to assert in debug mode.
    return GetEmptyImage();
}

}  // namespace ui
