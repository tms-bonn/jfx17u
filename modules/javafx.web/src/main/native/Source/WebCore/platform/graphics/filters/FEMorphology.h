/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "FilterEffect.h"

namespace WebCore {

enum class MorphologyOperatorType {
    Unknown,
    Erode,
    Dilate
};

class FEMorphology : public FilterEffect {
public:
    WEBCORE_EXPORT static Ref<FEMorphology> create(MorphologyOperatorType, float radiusX, float radiusY);

    MorphologyOperatorType morphologyOperator() const { return m_type; }
    bool setMorphologyOperator(MorphologyOperatorType);

    float radiusX() const { return m_radiusX; }
    bool setRadiusX(float);

    float radiusY() const { return m_radiusY; }
    bool setRadiusY(float);

private:
    FEMorphology(MorphologyOperatorType, float radiusX, float radiusY);

    FloatRect calculateImageRect(const Filter&, Span<const FloatRect> inputImageRects, const FloatRect& primitiveSubregion) const override;

    bool resultIsAlphaImage(const FilterImageVector& inputs) const override;

    std::unique_ptr<FilterEffectApplier> createSoftwareApplier() const override;

    WTF::TextStream& externalRepresentation(WTF::TextStream&, FilterRepresentation) const override;

    MorphologyOperatorType m_type;
    float m_radiusX;
    float m_radiusY;
};

} // namespace WebCore

namespace WTF {

template<> struct EnumTraits<WebCore::MorphologyOperatorType> {
    using values = EnumValues<
        WebCore::MorphologyOperatorType,

        WebCore::MorphologyOperatorType::Unknown,
        WebCore::MorphologyOperatorType::Erode,
        WebCore::MorphologyOperatorType::Dilate
    >;
};

} // namespace WTF

SPECIALIZE_TYPE_TRAITS_FILTER_EFFECT(FEMorphology)
