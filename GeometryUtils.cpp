#include "GeometryUtils.h"
#include "Core/math/Math.h"

GeometryUtils::GeometryUtils() {

}

Core::WeakPointer<Mesh> GeometryUtils::buildArrowMesh(Core::Real baseLength, Core::Real baseRadius,
                                                          Core::Real coneLength, Core::Real coneRadius,
                                                          Core::UInt32 subdivisions, Core::Color color) {

    Core::UInt32 facesPerSide = 6;
    Core::UInt32 faceCount = subdivisions * facesPerSide;
    Core::UInt32 verticesPerSide = facesPerSide * 3;
    Core::UInt32 componentsPerSide = verticesPerSide * 3;
    Core::Real vertices[faceCount * 3 * 3];

    Core::Real lastBaseX = 0.0f;
    Core::Real lastBaseY = 0.0f;
    Core::Real lastConeX = 0.0f;
    Core::Real lastConeY = 0.0f;
    for (Core::UInt32 s = 0; s < subdivisions; s++) {
        Core::Real t = static_cast<Core::Real>(s) / static_cast<Core::Real>(subdivisions);
        float angle = t * Core::Math::TwoPI;
        Core::Real x = Core::Math::cos(angle);
        Core::Real y = Core::Math::sin(angle);
        Core::Real baseX = x * baseRadius;
        Core::Real baseY = y * baseRadius;
        Core::Real coneX = x * coneRadius;
        Core::Real coneY = y * coneRadius;

        if (s >= 1) {
            Core::UInt32 index = s * componentsPerSide;

            // bottom of base
            // Core::Vector3r botA(baseX, 0.0, baseY);
            // Core::Vector3r botB(lastBaseX, 0.0, lastBaseY);
            // Core::Vector3r botC(0.0, 0.0, 0.0);
            vertices[index] = baseX;
            vertices[index + 1] = 0.0;
            vertices[index + 2] = baseY;
            vertices[index + 3] = lastBaseX;
            vertices[index + 4] = 0.0;
            vertices[index + 5] = lastBaseY;
            vertices[index + 6] = 0.0;
            vertices[index + 7] = 0.0;
            vertices[index + 8] = 0.0;

            // side of base
            //Core::Vector3r sideA(lastBaseX, 0.0, lastBaseY);
            //Core::Vector3r sideB(baseX, 0.0, baseY);
            //Core::Vector3r sideC(baseX, baseLength, baseY);
            //Core::Vector3r sideD(baseX, baseLength, baseY);
            //Core::Vector3r sideE(lastBaseX, baseLength, lastBaseY);
            //Core::Vector3r sideF(lastBaseX, 0.0, lastBaseY);
            vertices[index + 9] = lastBaseX;
            vertices[index + 10] = 0.0;
            vertices[index + 11] = lastBaseY;
            vertices[index + 12] = baseX;
            vertices[index + 13] = 0.0;
            vertices[index + 14] = baseY;
            vertices[index + 15] = baseX;
            vertices[index + 16] = baseLength;
            vertices[index + 17] = baseY;

            vertices[index + 18] = baseX;
            vertices[index + 19] = baseLength;
            vertices[index + 20] = baseY;
            vertices[index + 21] = lastBaseX;
            vertices[index + 22] = baseLength;
            vertices[index + 23] = lastBaseY;
            vertices[index + 24] = lastBaseX;
            vertices[index + 25] = 0.0;
            vertices[index + 26] = lastBaseY;


            // bottom of cone
            //Core::Vector3r botConeA(lastBaseX, baseLength, lastBaseY);
            //Core::Vector3r botConeB(baseX, baseLength, baseY);
            //Core::Vector3r botConeC(coneX, baseLength, coneY);
            //Core::Vector3r botConeD(coneX, baseLength, coneY);
            //Core::Vector3r botConeE(lastConeX, baseLength, lastConeY);
            //Core::Vector3r botConeF(lastBaseX, baseLength, lastBaseY);

            vertices[index + 27] = lastBaseX;
            vertices[index + 28] = baseLength;
            vertices[index + 29] = lastBaseY;
            vertices[index + 30] = baseX;
            vertices[index + 31] = baseLength;
            vertices[index + 32] = baseY;
            vertices[index + 33] = coneX;
            vertices[index + 34] = baseLength;
            vertices[index + 35] = coneY;

            vertices[index + 36] = coneX;
            vertices[index + 37] = baseLength;
            vertices[index + 38] = coneY;
            vertices[index + 39] = lastConeX;
            vertices[index + 40] = baseLength;
            vertices[index + 41] = lastConeY;
            vertices[index + 42] = lastBaseX;
            vertices[index + 43] = baseLength;
            vertices[index + 44] = lastBaseY;

            // cone face
            //Core::Vector3r coneA(lastConeX, baseLength, lastConeY);
            //Core::Vector3r coneB(coneX, baseLength, coneY);
            //Core::Vector3r coneC(0.0, baseLength + coneLength, 0.0);
            vertices[index + 45] = lastConeX;
            vertices[index + 46] = baseLength;
            vertices[index + 47] = lastConeY;
            vertices[index + 48] = coneX;
            vertices[index + 49] = baseLength;
            vertices[index + 50] = coneY;
            vertices[index + 51] = 0.0;
            vertices[index + 52] = baseLength + coneLength;
            vertices[index + 53] = 0.0;
        }

        lastBaseX = baseX;
        lastBaseY = baseY;
        lastConeX = coneX;
        lastConeY = coneY;
    }

}
