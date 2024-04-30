#include "OSGMaterialStruct.h"

#include "gui_helper_functions.h"

namespace mars
{
    namespace graphics
    {

        OSGMaterialStruct::OSGMaterialStruct(const mars::interfaces::MaterialData &mat)
        {
            setColorMode(osg::Material::OFF);
            setAmbient(osg::Material::FRONT, toOSGVec4(mat.ambientFront));
            setAmbient(osg::Material::BACK, toOSGVec4(mat.ambientBack));
            setSpecular(osg::Material::FRONT, toOSGVec4(mat.specularFront));
            setSpecular(osg::Material::BACK, toOSGVec4(mat.specularBack));
            setDiffuse(osg::Material::FRONT, toOSGVec4(mat.diffuseFront));
            setDiffuse(osg::Material::BACK, toOSGVec4(mat.diffuseBack));
            setEmission(osg::Material::FRONT, toOSGVec4(mat.emissionFront));
            setEmission(osg::Material::BACK, toOSGVec4(mat.emissionBack));
            setShininess(osg::Material::FRONT_AND_BACK, mat.shininess);
            setTransparency(osg::Material::FRONT_AND_BACK, mat.transparency);
        }

    } // end of namespace graphics
} // end of namespace mars
