#include "ray.h"
#include "material.h"
#include "light.h"

#include "../fileio/imageio.h"

using namespace std;
extern bool debugMode;

// Apply the Phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the Phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
	if( debugMode )
		std::cout << "Debugging the Phong code (or lack thereof...)" << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( vector<Light*>::const_iterator litr = scene->beginLights(); 
	// 		litr != scene->endLights(); 
	// 		++litr )
	// {
	// 		Light* pLight = *litr;
	// 		.
	// 		.
	// 		.
	// }

																				//**** �ǉ��ӏ� ****//
	Vec3d lum = ke(i) + prod(ka(i), scene->ambient());	  // �Ԃ�l
	Vec3d v = -r.getDirection();	// �����x�N�g����Ray�̕����x�N�g��d�Ɠ�����

	for (vector<Light*>::const_iterator litr = scene->beginLights();
		litr != scene->endLights(); litr++) {	// �S�Ă̌����Ɋւ��ă��[�v
		Light* pLight = *litr;		// ���̃��[�v�Œ��ڂ��������pLight�Ƃ���
		Vec3d ld = pLight->getDirection(r.at(i.t));
		ld.normalize();

		Vec3d lc = pLight->getColor(r.at(i.t));	// pLight����o����̗�
		Vec3d atten;		// ������. ��̂��߂Ƀx�N�g���^�ɂ��Ă�����
		//atten = pLight->distanceAttenuation(r.at(i.t)) * Vec3d(1.0, 1.0, 1.0);
		atten = pLight->distanceAttenuation(r.at(i.t)) * pLight->shadowAttenuation(r.at(i.t));	// �e�ɂ�錸��(��2�񉉏K�ɂĕύX)
		
		// diffuse color
		Vec3d ldf;		// �������甽�˂���g�U���̒l
		/* ldf�̒l�������Ōv�Z���悤 */
		ldf = kd(i)*max(0, i.N*ld);		//0��i.N*ld�̑傫���� ��Ԃ�

		// specular color
		Vec3d lsp;		// �������甽�˂��鋾�ʌ��̒l
		/* lsp�̒l�������Ōv�Z���悤 */
		Vec3d R = ld + 2 * ((i.N*ld)*i.N - ld);

		Vec3d V = -r.getDirection();
		double ns = shininess(i);
		lsp = ks(i)*pow(max(0, R*V), ns);       //pow(a,b)  : a��b���Ԃ�

		lum += prod(atten, prod(lc, ldf + lsp));
	}
	return lum;

//	return kd(i);

}


TextureMap::TextureMap( string filename )
{
    data = load( filename.c_str(), width, height );
    if( 0 == data )
    {
        width = 0;
        height = 0;
        string error( "Unable to load texture map '" );
        error.append( filename );
        error.append( "'." );
        throw TextureMapException( error );
    }
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{
	// YOUR CODE HERE

    // In order to add texture mapping support to the 
    // raytracer, you need to implement this function.
    // What this function should do is convert from
    // parametric space which is the unit square
    // [0, 1] x [0, 1] in 2-space to Image coordinates,
    // and use these to perform bilinear interpolation
    // of the values.



    //return Vec3d(1.0, 1.0, 1.0);
	return getPixelAt(coord[0] * width, coord[1] * height);			//**** �ǉ��ӏ�(�e�N�X�`���I�v�V����) ****//
}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d( double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0 );
}

Vec3d MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

