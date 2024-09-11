/*
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
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
#include "CryptoAlgorithmRSA_PSS.h"

#if ENABLE(WEB_CRYPTO) && HAVE(RSA_PSS)

#include "CryptoAlgorithmRsaPssParams.h"
#include "CryptoKeyRSA.h"
#include "OpenSSLUtilities.h"

namespace WebCore {

ExceptionOr<Vector<uint8_t>> CryptoAlgorithmRSA_PSS::platformSign(const CryptoAlgorithmRsaPssParams& parameters, const CryptoKeyRSA& key, const Vector<uint8_t>& data)
{
#if defined(EVP_PKEY_CTX_set_rsa_pss_saltlen) && defined(EVP_PKEY_CTX_set_rsa_mgf1_md)
    const EVP_MD* md = digestAlgorithm(key.hashAlgorithmIdentifier());
    if (!md)
        return Exception { ExceptionCode::NotSupportedError };

    std::optional<Vector<uint8_t>> digest = calculateDigest(md, data);
    if (!digest)
        return Exception { ExceptionCode::OperationError };

    auto ctx = EvpPKeyCtxPtr(EVP_PKEY_CTX_new(key.platformKey(), nullptr));
    if (!ctx)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_sign_init(ctx.get()) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PSS_PADDING ) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx.get(), parameters.saltLength) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_signature_md(ctx.get(), md) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), md) <= 0)
        return Exception { ExceptionCode::OperationError };

    size_t signatureLen;
    if (EVP_PKEY_sign(ctx.get(), nullptr, &signatureLen, digest->data(), digest->size()) <= 0)
        return Exception { ExceptionCode::OperationError };

    Vector<uint8_t> signature(signatureLen);
    if (EVP_PKEY_sign(ctx.get(), signature.data(), &signatureLen, digest->data(), digest->size()) <= 0)
        return Exception { ExceptionCode::OperationError };
    signature.shrink(signatureLen);

    return signature;
#else
    return Exception { ExceptionCode::NotSupportedError };
#endif
}

ExceptionOr<bool> CryptoAlgorithmRSA_PSS::platformVerify(const CryptoAlgorithmRsaPssParams& parameters, const CryptoKeyRSA& key, const Vector<uint8_t>& signature, const Vector<uint8_t>& data)
{
#if defined(EVP_PKEY_CTX_set_rsa_pss_saltlen) && defined(EVP_PKEY_CTX_set_rsa_mgf1_md)
    const EVP_MD* md = digestAlgorithm(key.hashAlgorithmIdentifier());
    if (!md)
        return Exception { ExceptionCode::NotSupportedError };

    std::optional<Vector<uint8_t>> digest = calculateDigest(md, data);
    if (!digest)
        return Exception { ExceptionCode::OperationError };

    auto ctx = EvpPKeyCtxPtr(EVP_PKEY_CTX_new(key.platformKey(), nullptr));
    if (!ctx)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_verify_init(ctx.get()) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PSS_PADDING ) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx.get(), parameters.saltLength) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_signature_md(ctx.get(), md) <= 0)
        return Exception { ExceptionCode::OperationError };

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx.get(), md) <= 0)
        return Exception { ExceptionCode::OperationError };

    int ret = EVP_PKEY_verify(ctx.get(), signature.data(), signature.size(), digest->data(), digest->size());

    return ret == 1;
#else
    return Exception { ExceptionCode::NotSupportedError };
#endif
}

} // namespace WebCore

#endif // ENABLE(WEB_CRYPTO)
