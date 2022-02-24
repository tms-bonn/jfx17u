/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ReftestFunctions.h"

#include "JSBasics.h"

namespace WTR {

void sendTestRenderedEvent(JSGlobalContextRef context)
{
    if (!context)
        return;
    auto initializer = JSObjectMake(context, nullptr, nullptr);
    setProperty(context, initializer, "bubbles", true);
    auto value = makeValue(context, "TestRendered");
    JSValueProtect(context, value);
    auto event = callConstructor(context, "Event", { value, initializer });
    auto documentElement = objectProperty(context, JSContextGetGlobalObject(context), { "document", "documentElement" });
    call(context, documentElement, "dispatchEvent", { event });
    JSValueUnprotect(context, value);
}

bool hasReftestWaitAttribute(JSGlobalContextRef context)
{
    if (!context)
        return false;
    auto classList = objectProperty(context, JSContextGetGlobalObject(context), { "document", "documentElement", "classList" });
    return JSValueToBoolean(context, call(context, classList, "contains", { makeValue(context, "reftest-wait") }));
}

}
