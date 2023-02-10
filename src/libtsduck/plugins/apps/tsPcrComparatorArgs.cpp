//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2022, Thierry Lelegard
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

#include "tsPcrComparatorArgs.h"
#include "tsArgsWithPlugins.h"

#if defined(TS_NEED_STATIC_CONST_DEFINITIONS)
constexpr size_t ts::PcrComparatorArgs::DEFAULT_MAX_INPUT_PACKETS;
constexpr size_t ts::PcrComparatorArgs::MIN_INPUT_PACKETS;
constexpr size_t ts::PcrComparatorArgs::DEFAULT_BUFFERED_PACKETS;
constexpr size_t ts::PcrComparatorArgs::MIN_BUFFERED_PACKETS;
constexpr int ts::PcrComparatorArgs::DESIGNATED_INPUT_PLUGIN_NUMBER;
#endif


//----------------------------------------------------------------------------
// Constructors.
//----------------------------------------------------------------------------

ts::PcrComparatorArgs::PcrComparatorArgs() :
    appName(),
    bufferedPackets(0),
    maxInputPackets(0),
    inputs(),
    output_name()
{
}


//----------------------------------------------------------------------------
// Enforce default or minimum values.
//----------------------------------------------------------------------------

void ts::PcrComparatorArgs::enforceDefaults()
{
    if (inputs.empty()) {
        // If no input plugin is used, used only standard input.
        inputs.push_back(PluginOptions(u"file"));
    }

    bufferedPackets = std::max(bufferedPackets, MIN_BUFFERED_PACKETS);
    maxInputPackets = std::max(maxInputPackets, MIN_INPUT_PACKETS);
}


//----------------------------------------------------------------------------
// Define command line options in an Args.
//----------------------------------------------------------------------------

void ts::PcrComparatorArgs::defineArgs(Args& args)
{
    args.option(u"buffer-packets", 'b', Args::POSITIVE);
    args.help(u"buffer-packets",
              u"Specify the size in TS packets of each input plugin buffer. "
              u"The default is " + UString::Decimal(DEFAULT_BUFFERED_PACKETS) + u" packets.");

    args.option(u"max-input-packets", 0, Args::POSITIVE);
    args.help(u"max-input-packets",
              u"Specify the maximum number of TS packets to read at a time. "
              u"This value may impact the switch response time. "
              u"The default is " + UString::Decimal(DEFAULT_MAX_INPUT_PACKETS) + u" packets. "
              u"The actual value is never more than half the --buffer-packets value.");

    args.option(u"output-file", 'o', Args::FILENAME);
    args.help(u"output-file", u"filename",
              u"Output file name for CSV reporting (standard error by default).");
    
    args.option(u"separator", 's', Args::STRING);
    args.help(u"separator", u"string",
              u"Field separator string in CSV output (default: '" TS_DEFAULT_CSV_SEPARATOR u"').");
}


//----------------------------------------------------------------------------
// Load arguments from command line.
//----------------------------------------------------------------------------

bool ts::PcrComparatorArgs::loadArgs(Args& args)
{
    appName = args.appName();
    args.getIntValue(bufferedPackets, u"buffer-packets", DEFAULT_BUFFERED_PACKETS);
    maxInputPackets = std::min(args.intValue<size_t>(u"max-input-packets", DEFAULT_MAX_INPUT_PACKETS), bufferedPackets / 2);
    output_name = args.value(u"output-file");

    // Load all plugin descriptions. Default output is the standard output file.
    ArgsWithPlugins* pargs = dynamic_cast<ArgsWithPlugins*>(&args);
    if (pargs != nullptr) {
        pargs->getPlugins(inputs, PluginType::INPUT);
    }
    else {
        inputs.clear();
    }
    if (inputs.empty()) {
        // If no input plugin is used, used only standard input.
        inputs.push_back(PluginOptions(u"file"));
    }

    // Check number of input plugins
    if (inputs.size() != DESIGNATED_INPUT_PLUGIN_NUMBER) {
        args.error(u"Number of input plugins must be %d", {DESIGNATED_INPUT_PLUGIN_NUMBER});
    }

    return args.valid();
}
