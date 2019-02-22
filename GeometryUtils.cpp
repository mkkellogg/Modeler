#include "GeometryUtils.h"
#include "Core/math/Math.h"
#include "Core/material/StandardAttributes.h"
#include "Core/render/MeshRenderer.h"

using MeshContainer = Core::RenderableContainer<Core::Mesh>;

GeometryUtils::GeometryUtils() {

}

Core::WeakPointer<Core::Mesh> GeometryUtils::buildBoxMesh(Core::Real length, Core::Real height, Core::Real depth, Core::Color color) {

    Core::Real halfLength = length / 2.0f;
    Core::Real halfHeight = height / 2.0f;
    Core::Real halfDepth = depth / 2.0f;

    Core::Real vertexPositions[] = {
        // back
        -halfLength, -halfHeight, -halfDepth, 1.0, halfLength, halfHeight, -halfDepth, 1.0, -halfLength, halfHeight, -halfDepth, 1.0,
        -halfLength, -halfHeight, -halfDepth, 1.0, halfLength, -halfHeight, -halfDepth, 1.0, halfLength, halfHeight, -halfDepth, 1.0,
        // left
        -halfLength, -halfHeight, -halfDepth, 1.0, -halfLength, halfHeight, -halfDepth, 1.0, -halfLength, -halfHeight, halfDepth, 1.0,
        -halfLength, halfHeight, -halfDepth, 1.0, -halfLength, halfHeight, halfDepth, 1.0, -halfLength, -halfHeight, halfDepth, 1.0,
        // right
        halfLength, -halfHeight, -halfDepth, 1.0, halfLength, -halfHeight, halfDepth, 1.0, halfLength, halfHeight, -halfDepth, 1.0,
        halfLength, halfHeight, -halfDepth, 1.0, halfLength, -halfHeight, halfDepth, 1.0, halfLength, halfHeight, halfDepth, 1.0,
        // top
        -halfLength, halfHeight, -halfDepth, 1.0, halfLength, halfHeight, -halfDepth, 1.0, -halfLength, halfHeight, halfDepth, 1.0,
        halfLength, halfHeight, -halfDepth, 1.0, halfLength, halfHeight, halfDepth, 1.0, -halfLength, halfHeight, halfDepth, 1.0,
        // bottom
        -halfLength, -halfHeight, -halfDepth, 1.0, -halfLength, -halfHeight, halfDepth, 1.0, halfLength, -halfHeight, halfDepth, 1.0,
        -halfLength, -halfHeight, -halfDepth, 1.0, halfLength, -halfHeight, halfDepth, 1.0, halfLength, -halfHeight, -halfDepth, 1.0,
        // front
        halfLength, halfHeight, halfDepth, 1.0, -halfLength, -halfHeight, halfDepth, 1.0, -halfLength, halfHeight, halfDepth, 1.0,
        halfLength, halfHeight, halfDepth, 1.0, halfLength, -halfHeight, halfDepth, 1.0, -halfLength, -halfHeight, halfDepth, 1.0
    };

    Core::Real vertexNormals[] = {
        // back
        0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
        0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
        // left
        -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
        -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
        // right
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        // top
        0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        // bottom
        0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
        0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
        // front
        0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
    };

    Core::Real vertexColors[] = {
        // back
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        // left
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        // right
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        // top
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        // bottom
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        // front
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a,
        color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a, color.r, color.g, color.b, color.a
    };

    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    Core::WeakPointer<Core::Mesh> boxMesh(engine->createMesh(36, false));
    boxMesh->init();
    boxMesh->enableAttribute(Core::StandardAttribute::Position);
    Core::Bool positionInited = boxMesh->initVertexPositions();
    ASSERT(positionInited, "Unable to initializebox mesh vertex positions.");
    boxMesh->getVertexPositions()->store(vertexPositions);

    boxMesh->enableAttribute(Core::StandardAttribute::Color);
    Core::Bool colorInited = boxMesh->initVertexColors();
    ASSERT(colorInited, "Unable to initialize box mesh colors.");
    boxMesh->getVertexColors()->store(vertexColors);

    boxMesh->enableAttribute(Core::StandardAttribute::Normal);
    Core::Bool normalInited = boxMesh->initVertexNormals();
    ASSERT(normalInited, "Unable to initialize box vertex normals.");
    boxMesh->getVertexNormals()->store(vertexNormals);

    boxMesh->enableAttribute(Core::StandardAttribute::FaceNormal);
    Core::Bool faceNormalInited = boxMesh->initVertexFaceNormals();

    boxMesh->calculateBoundingBox();

    return boxMesh;
}

Core::WeakPointer<Core::Mesh> GeometryUtils::buildArrowMesh(Core::Real baseLength, Core::Real baseRadius,
                                                               Core::Real coneLength, Core::Real coneRadius,
                                                               Core::UInt32 subdivisions, Core::Color color) {

    Core::UInt32 facesPerSide = 6;
    Core::UInt32 faceCount = subdivisions * facesPerSide;
    Core::UInt32 verticesPerSide = facesPerSide * 3;
    Core::UInt32 componentsPerSide = verticesPerSide * 4;
    Core::UInt32 vertexCount = faceCount * 3;
    Core::UInt32 componentCount = vertexCount * 4;
    Core::Real vertices[componentCount];
    Core::Real colors[componentCount];

    Core::Real lastBaseX = 0.0f;
    Core::Real lastBaseY = 0.0f;
    Core::Real lastConeX = 0.0f;
    Core::Real lastConeY = 0.0f;
    for (Core::UInt32 s = 0; s <= subdivisions; s++) {
        Core::Real t = static_cast<Core::Real>(s) / static_cast<Core::Real>(subdivisions);
        float angle = t * Core::Math::TwoPI;
        Core::Real x = Core::Math::cos(angle);
        Core::Real y = Core::Math::sin(angle);
        if (s == subdivisions) {
            x = 1.0f;
            y = 0.0;
        }

        Core::Real baseX = x * baseRadius;
        Core::Real baseY = y * baseRadius;
        Core::Real coneX = x * coneRadius;
        Core::Real coneY = y * coneRadius;


        if (s >= 1) {
            Core::UInt32 index = (s - 1) * componentsPerSide;

            // bottom of base
            // Core::Vector3r botA(baseX, 0.0, baseY);
            // Core::Vector3r botB(lastBaseX, 0.0, lastBaseY);
            // Core::Vector3r botC(0.0, 0.0, 0.0);
            vertices[index] = baseX;
            vertices[index + 1] = 0.0;
            vertices[index + 2] = baseY;
            vertices[index + 3] = 1.0;
            vertices[index + 4] = lastBaseX;
            vertices[index + 5] = 0.0;
            vertices[index + 6] = lastBaseY;
            vertices[index + 7] = 1.0;
            vertices[index + 8] = 0.0;
            vertices[index + 9] = 0.0;
            vertices[index + 10] = 0.0;
            vertices[index + 11] = 1.0;

            // side of base
            //Core::Vector3r sideA(lastBaseX, 0.0, lastBaseY);
            //Core::Vector3r sideB(baseX, 0.0, baseY);
            //Core::Vector3r sideC(baseX, baseLength, baseY);
            //Core::Vector3r sideD(baseX, baseLength, baseY);
            //Core::Vector3r sideE(lastBaseX, baseLength, lastBaseY);
            //Core::Vector3r sideF(lastBaseX, 0.0, lastBaseY);
            vertices[index + 12] = lastBaseX;
            vertices[index + 13] = 0.0;
            vertices[index + 14] = lastBaseY;
            vertices[index + 15] = 1.0;
            vertices[index + 16] = baseX;
            vertices[index + 17] = 0.0;
            vertices[index + 18] = baseY;
            vertices[index + 19] = 1.0;
            vertices[index + 20] = baseX;
            vertices[index + 21] = baseLength;
            vertices[index + 22] = baseY;
            vertices[index + 23] = 1.0;

            vertices[index + 24] = baseX;
            vertices[index + 25] = baseLength;
            vertices[index + 26] = baseY;
            vertices[index + 27] = 1.0;
            vertices[index + 28] = lastBaseX;
            vertices[index + 29] = baseLength;
            vertices[index + 30] = lastBaseY;
            vertices[index + 31] = 1.0;
            vertices[index + 32] = lastBaseX;
            vertices[index + 33] = 0.0;
            vertices[index + 34] = lastBaseY;
            vertices[index + 35] = 1.0;


            // bottom of cone
            //Core::Vector3r botConeA(lastBaseX, baseLength, lastBaseY);
            //Core::Vector3r botConeB(baseX, baseLength, baseY);
            //Core::Vector3r botConeC(coneX, baseLength, coneY);
            //Core::Vector3r botConeD(coneX, baseLength, coneY);
            //Core::Vector3r botConeE(lastConeX, baseLength, lastConeY);
            //Core::Vector3r botConeF(lastBaseX, baseLength, lastBaseY);

            vertices[index + 36] = lastBaseX;
            vertices[index + 37] = baseLength;
            vertices[index + 38] = lastBaseY;
            vertices[index + 39] = 1.0;
            vertices[index + 40] = baseX;
            vertices[index + 41] = baseLength;
            vertices[index + 42] = baseY;
            vertices[index + 43] = 1.0;
            vertices[index + 44] = coneX;
            vertices[index + 45] = baseLength;
            vertices[index + 46] = coneY;
            vertices[index + 47] = 1.0;

            vertices[index + 48] = coneX;
            vertices[index + 49] = baseLength;
            vertices[index + 50] = coneY;
            vertices[index + 51] = 1.0;
            vertices[index + 52] = lastConeX;
            vertices[index + 53] = baseLength;
            vertices[index + 54] = lastConeY;
            vertices[index + 55] = 1.0;
            vertices[index + 56] = lastBaseX;
            vertices[index + 57] = baseLength;
            vertices[index + 58] = lastBaseY;
            vertices[index + 59] = 1.0;

            // cone face
            //Core::Vector3r coneA(lastConeX, baseLength, lastConeY);
            //Core::Vector3r coneB(coneX, baseLength, coneY);
            //Core::Vector3r coneC(0.0, baseLength + coneLength, 0.0);
            vertices[index + 60] = lastConeX;
            vertices[index + 61] = baseLength;
            vertices[index + 62] = lastConeY;
            vertices[index + 63] = 1.0;
            vertices[index + 64] = coneX;
            vertices[index + 65] = baseLength;
            vertices[index + 66] = coneY;
            vertices[index + 67] = 1.0;
            vertices[index + 68] = 0.0;
            vertices[index + 69] = baseLength + coneLength;
            vertices[index + 70] = 0.0;
            vertices[index + 71] = 1.0;
        }

        for (Core::UInt32 v = 0; v < vertexCount; v++) {
            Core::UInt32 index = v * 4;
            colors[index] = color.r;
            colors[index + 1] = color.g;
            colors[index + 2] = color.b;
            colors[index + 3] = color.a;
        }

        lastBaseX = baseX;
        lastBaseY = baseY;
        lastConeX = coneX;
        lastConeY = coneY;
    }

    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    Core::WeakPointer<Core::Mesh> arrowMesh(engine->createMesh(vertexCount, false));
    arrowMesh->init();
    arrowMesh->enableAttribute(Core::StandardAttribute::Position);
    Core::Bool positionInited = arrowMesh->initVertexPositions();
    ASSERT(positionInited, "Unable to initialize arrow mesh vertex positions.");
    arrowMesh->getVertexPositions()->store(vertices);

    arrowMesh->enableAttribute(Core::StandardAttribute::Color);
    Core::Bool colorInited = arrowMesh->initVertexColors();
    ASSERT(colorInited, "Unable to initialize arrow mesh colors.");
    arrowMesh->getVertexColors()->store(colors);

    arrowMesh->enableAttribute(Core::StandardAttribute::Normal);
    Core::Bool normalInited = arrowMesh->initVertexNormals();
    ASSERT(normalInited, "Unable to initialize arrow vertex normals.");

    arrowMesh->enableAttribute(Core::StandardAttribute::FaceNormal);
    Core::Bool faceNormalInited = arrowMesh->initVertexFaceNormals();

    arrowMesh->calculateBoundingBox();
    arrowMesh->calculateNormals(75.0f);

    return arrowMesh;
}

Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> GeometryUtils::buildMeshContainer(Core::WeakPointer<Core::Mesh> mesh,
                                                                       Core::WeakPointer<Core::Material> material,
                                                                       const std::string& name) {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();
    Core::WeakPointer<MeshContainer> obj(engine->createObject3D<MeshContainer>());
    obj->setName(name);
    Core::WeakPointer<Core::MeshRenderer> renderer(engine->createRenderer<Core::MeshRenderer>(material, obj));
    obj->addRenderable(mesh);
    return obj;
}
