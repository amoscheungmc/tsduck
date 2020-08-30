//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#include "tsEutelsatChannelNumberDescriptor.h"
#include "tsDescriptor.h"
#include "tsTablesDisplay.h"
#include "tsPSIRepository.h"
#include "tsPSIBuffer.h"
#include "tsDuckContext.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"eutelsat_channel_number_descriptor"
#define MY_CLASS ts::EutelsatChannelNumberDescriptor
#define MY_DID ts::DID_EUTELSAT_CHAN_NUM
#define MY_PDS ts::PDS_EUTELSAT
#define MY_STD ts::Standards::DVB

TS_REGISTER_DESCRIPTOR(MY_CLASS, ts::EDID::Private(MY_DID, MY_PDS), MY_XML_NAME, MY_CLASS::DisplayDescriptor);


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::EutelsatChannelNumberDescriptor::EutelsatChannelNumberDescriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, MY_PDS),
    entries()
{
}

ts::EutelsatChannelNumberDescriptor::EutelsatChannelNumberDescriptor(DuckContext& duck, const Descriptor& desc) :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, MY_PDS),
    entries()
{
    deserialize(duck, desc);
}

void ts::EutelsatChannelNumberDescriptor::clearContent()
{
    entries.clear();
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::EutelsatChannelNumberDescriptor::serializePayload(PSIBuffer& buf) const
{
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        buf.putUInt16(it->onetw_id);
        buf.putUInt16(it->ts_id);
        buf.putUInt16(it->service_id);
        buf.putBits(0xFF, 4);
        buf.putBits(it->ecn, 12);
    }
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::EutelsatChannelNumberDescriptor::deserializePayload(PSIBuffer& buf)
{
    while (!buf.error() && !buf.endOfRead()) {
        Entry e;
        e.onetw_id = buf.getUInt16();
        e.ts_id = buf.getUInt16();
        e.service_id = buf.getUInt16();
        buf.skipBits(4);
        e.ecn = buf.getBits<uint16_t>(12);
        entries.push_back(e);
    }
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::EutelsatChannelNumberDescriptor::DisplayDescriptor(TablesDisplay& disp, PSIBuffer& buf, const UString& margin, DID did, TID tid, PDS pds)
{
    while (!buf.error() && buf.remainingReadBytes() >= 8) {
        const uint16_t onetw_id = buf.getUInt16();
        const uint16_t ts_id = buf.getUInt16();
        const uint16_t service_id = buf.getUInt16();
        buf.skipBits(4);
        const uint16_t channel = buf.getBits<uint16_t>(12);
        disp << margin
             << UString::Format(u"Service Id: %5d (0x%04<X), Channel number: %3d, TS Id: %5d (0x%<04X), Net Id: %5d (0x%<04X)", {service_id, channel, ts_id, onetw_id})
             << std::endl;
    }
    disp.displayExtraData(buf, margin);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::EutelsatChannelNumberDescriptor::buildXML(DuckContext& duck, xml::Element* root) const
{
    for (EntryList::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        xml::Element* e = root->addElement(u"service");
        e->setIntAttribute(u"original_network_id", it->onetw_id, true);
        e->setIntAttribute(u"transport_stream_id", it->ts_id, true);
        e->setIntAttribute(u"service_id", it->service_id, true);
        e->setIntAttribute(u"eutelsat_channel_number", it->ecn, false);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

bool ts::EutelsatChannelNumberDescriptor::analyzeXML(DuckContext& duck, const xml::Element* element)
{
    xml::ElementVector children;
    bool ok = element->getChildren(children, u"service", 0, MAX_ENTRIES);

    for (size_t i = 0; ok && i < children.size(); ++i) {
        Entry entry;
        ok =
            children[i]->getIntAttribute<uint16_t>(entry.onetw_id, u"original_network_id", true, 0, 0x0000, 0xFFFF) &&
            children[i]->getIntAttribute<uint16_t>(entry.ts_id, u"transport_stream_id", true, 0, 0x0000, 0xFFFF) &&
            children[i]->getIntAttribute<uint16_t>(entry.service_id, u"service_id", true, 0, 0x0000, 0xFFFF) &&
            children[i]->getIntAttribute<uint16_t>(entry.ecn, u"eutelsat_channel_number", true, 0, 0x0000, 0x03FF);
        entries.push_back(entry);
    }
    return ok;
}
