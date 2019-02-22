#pragma once

#include "Core/Engine.h"
#include "Core/util/WeakPointer.h"
#include "Core/material/Material.h"

class BasicRimShadowMaterial : public Core::Material {
    friend class Core::Engine;

public:
    virtual Core::Bool build() override;
    virtual Core::Int32 getShaderLocation(Core::StandardAttribute attribute, Core::UInt32 offset = 0) override;
    virtual Core::Int32 getShaderLocation(Core::StandardUniform uniform, Core::UInt32 offset = 0) override;
    virtual void sendCustomUniformsToShader() override;
    virtual Core::WeakPointer<Material> clone() override;

    void setHighlightLowerBound(Core::Real lowerBound);
    void setHighlightScale(Core::Real scale);
    void setHighlightColor(Core::Color color);

private:
    BasicRimShadowMaterial(Core::WeakPointer<Core::Graphics> graphics);
    void bindShaderVarLocations();

    Core::Int32 positionLocation;
    Core::Int32 normalLocation;
    Core::Int32 faceNormalLocation;
    Core::Int32 colorLocation;
    Core::Int32 uvLocation;
    Core::Int32 projectionMatrixLocation;
    Core::Int32 viewMatrixLocation;
    Core::Int32 modelMatrixLocation;
    Core::Int32 modelInverseTransposeMatrixLocation;
    Core::Int32 highlightColorLocation;
    Core::Int32 highlightScaleLocation;
    Core::Int32 highlightLowerBoundLocation;

    Core::Color highlightColor;
    Core::Real highlightScale;
    Core::Real highlightLowerBound;
};
