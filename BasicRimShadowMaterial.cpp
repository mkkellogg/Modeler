#include "BasicRimShadowMaterial.h"
#include "Core/material/Shader.h"
#include "Core/util/WeakPointer.h"
#include "Core/material/StandardAttributes.h"
#include "Core/material/StandardUniforms.h"
#include "Core/Engine.h"

static auto _un = Core::StandardUniforms::getUniformName;
static auto _an = Core::StandardAttributes::getAttributeName;

std::string vertexShader =
   "#version 330\n"
   "precision highp float;\n"
   "in vec4 " + _an(Core::StandardAttribute::Position) + ";\n"
   "in vec4 " + _an(Core::StandardAttribute::Normal) + ";\n"
   "in vec4 " + _an(Core::StandardAttribute::FaceNormal) + ";\n"
   "in vec4 " + _an(Core::StandardAttribute::Color) + ";\n"
   "uniform mat4 " + _un(Core::StandardUniform::ProjectionMatrix) + ";\n"
   "uniform mat4 " + _un(Core::StandardUniform::ViewMatrix) + ";\n"
   "uniform mat4 " + _un(Core::StandardUniform::ModelMatrix) + ";\n"
   "uniform mat4 " + _un(Core::StandardUniform::ModelInverseTransposeMatrix) + ";\n"
   "out vec4 vColor;\n"
   "out vec3 vNormal;\n"
   "out vec3 vViewWorld;\n"
   "void main() {\n"
   "    vViewWorld = transpose(mat3(" + _un(Core::StandardUniform::ViewMatrix) + ")) * vec3(0.0, 0.0, 1.0);\n"
   "    gl_Position = " + _un(Core::StandardUniform::ProjectionMatrix) + "  * " + _un(Core::StandardUniform::ViewMatrix) + " * " +
        _un(Core::StandardUniform::ModelMatrix) + " * " + _an(Core::StandardAttribute::Position) + ";\n"
   "    vColor = " + _an(Core::StandardAttribute::Color) + ";\n"
   "    vNormal = vec3(" + _un(Core::StandardUniform::ModelInverseTransposeMatrix) + " * " + _an(Core::StandardAttribute::Normal) + ");\n"
   "}\n";

std::string fragmentShader =
   "#version 330\n"
   "precision highp float;\n"
   "uniform float highlightLowerBound;\n"
   "uniform float highlightScale;\n"
   "uniform vec4 highlightColor;\n"
   "in vec4 vColor;\n"
   "in vec3 vNormal;\n"
   "in vec3 vViewWorld;\n"
   "out vec4 out_color;\n"
   "void main() {\n"
   "    out_color = (highlightColor * vColor) * clamp(dot(normalize(vNormal), vViewWorld) * highlightScale, highlightLowerBound, 1.0);\n"
   "}\n";

BasicRimShadowMaterial::BasicRimShadowMaterial(Core::WeakPointer<Core::Graphics> graphics) : Core::Material(graphics) {
    this->highlightScale = 1.0f;
    this->highlightLowerBound = 0.0f;
    this->highlightColor.set(1.0f, 1.0f, 1.0f, 1.0f);
}

Core::Bool BasicRimShadowMaterial::build() {
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    const std::string& vertexSrc = vertexShader;
    const std::string& fragmentSrc = fragmentShader;
    Core::Bool ready = this->buildFromSource(vertexSrc, fragmentSrc);
    if (!ready) {
        return false;
    }

    this->bindShaderVarLocations();
    this->setLit(false);
    return true;
}

Core::Int32 BasicRimShadowMaterial::getShaderLocation(Core::StandardAttribute attribute, Core::UInt32 offset) {
    switch (attribute) {
        case Core::StandardAttribute::Position:
            return this->positionLocation;
        case Core::StandardAttribute::Normal:
            return this->normalLocation;
        case Core::StandardAttribute::FaceNormal:
            return this->faceNormalLocation;
        case Core::StandardAttribute::Color:
            return this->colorLocation;
        case Core::StandardAttribute::AlbedoUV:
            return this->uvLocation;
        default:
            return -1;
    }
}

Core::Int32 BasicRimShadowMaterial::getShaderLocation(Core::StandardUniform uniform, Core::UInt32 offset) {
    switch (uniform) {
        case Core::StandardUniform::ProjectionMatrix:
            return this->projectionMatrixLocation;
        case Core::StandardUniform::ViewMatrix:
            return this->viewMatrixLocation;
        case Core::StandardUniform::ModelMatrix:
            return this->modelMatrixLocation;
        case Core::StandardUniform::ModelInverseTransposeMatrix:
            return this->modelInverseTransposeMatrixLocation;
        default:
            return -1;
    }
}

void BasicRimShadowMaterial::sendCustomUniformsToShader() {
    this->shader->setUniform1f(this->highlightLowerBoundLocation, this->highlightLowerBound);
    this->shader->setUniform1f(this->highlightScaleLocation, this->highlightScale);
    this->shader->setUniform4f(this->highlightColorLocation, this->highlightColor.r, this->highlightColor.g, this->highlightColor.b, this->highlightColor.a);
}

Core::WeakPointer<Core::Material> BasicRimShadowMaterial::clone() {
    Core::WeakPointer<BasicRimShadowMaterial> newMaterial = Core::Engine::instance()->createMaterial<BasicRimShadowMaterial>(false);
    this->copyTo(newMaterial);
    newMaterial->positionLocation = this->positionLocation;
    newMaterial->normalLocation = this->normalLocation;
    newMaterial->faceNormalLocation = this->faceNormalLocation;
    newMaterial->colorLocation = this->colorLocation;
    newMaterial->projectionMatrixLocation = this->projectionMatrixLocation;
    newMaterial->viewMatrixLocation = this->viewMatrixLocation;
    newMaterial->modelMatrixLocation = this->modelMatrixLocation;
    newMaterial->modelInverseTransposeMatrixLocation = this->modelInverseTransposeMatrixLocation;
    newMaterial->highlightLowerBoundLocation = this->highlightLowerBoundLocation;
    newMaterial->highlightScaleLocation = this->highlightScaleLocation;
    newMaterial->highlightColorLocation = this->highlightColorLocation;
    return newMaterial;
}

void BasicRimShadowMaterial::setHighlightLowerBound(Core::Real lowerBound) {
    this->highlightLowerBound = lowerBound;
}

void BasicRimShadowMaterial::setHighlightScale(Core::Real scale) {
    this->highlightScale = scale;
}

void BasicRimShadowMaterial::setHighlightColor(Core::Color color) {
    this->highlightColor = color;
}

void BasicRimShadowMaterial::bindShaderVarLocations() {
    this->positionLocation = this->shader->getAttributeLocation(Core::StandardAttribute::Position);
    this->normalLocation = this->shader->getAttributeLocation(Core::StandardAttribute::Normal);
    this->faceNormalLocation = this->shader->getAttributeLocation(Core::StandardAttribute::FaceNormal);
    this->colorLocation = this->shader->getAttributeLocation(Core::StandardAttribute::Color);
    this->projectionMatrixLocation = this->shader->getUniformLocation(Core::StandardUniform::ProjectionMatrix);
    this->viewMatrixLocation = this->shader->getUniformLocation(Core::StandardUniform::ViewMatrix);
    this->modelMatrixLocation = this->shader->getUniformLocation(Core::StandardUniform::ModelMatrix);
    this->modelInverseTransposeMatrixLocation = this->shader->getUniformLocation(Core::StandardUniform::ModelInverseTransposeMatrix);
    this->highlightColorLocation = this->shader->getUniformLocation("highlightColor");
    this->highlightLowerBoundLocation = this->shader->getUniformLocation("highlightLowerBound");
    this->highlightScaleLocation = this->shader->getUniformLocation("highlightScale");
}
