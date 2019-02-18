#include "ray.h"
#include "material.h"
#include "light.h"
#include <math.h>
#include <fstream>

#include "../fileio/imageio.h"

#define PI 3.14159

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

	if (kr(i).n[0] >= 100 || kr(i).n[1] >= 100 || kr(i).n[2] >= 100)	return kr(i) / 100;		// indexVec デバッグ画面に印をつける(仮) reflective = (100,0,0);

																				//**** 追加箇所 ****//
	Vec3d lum = ke(i) + prod(ka(i), scene->ambient());	  // 返り値
	Vec3d v = -r.getDirection();	// 視線ベクトルはRayの方向ベクトルdと等しい

	for (vector<Light*>::const_iterator litr = scene->beginLights();
		litr != scene->endLights(); litr++) {	// 全ての光源に関してループ
		Light* pLight = *litr;		// このループで注目する光源をpLightとする
		Vec3d ld = pLight->getDirection(r.at(i.t));		// 光源方向の単位ベクトル
		ld.normalize();

		Vec3d lc = pLight->getColor(r.at(i.t));	// pLightから出る光の量
		Vec3d atten;		// 減衰率. 後のためにベクトル型にしておこう
		//atten = pLight->distanceAttenuation(r.at(i.t)) * Vec3d(1.0, 1.0, 1.0);
		atten = pLight->distanceAttenuation(r.at(i.t)) * pLight->shadowAttenuation(r.at(i.t));	// 影による減衰(第2回演習にて変更)
		
		// diffuse color
		Vec3d ldf;		// 光源から反射する拡散光の値
		/* ldfの値を自分で計算しよう */
		ldf = kd(i)*max(0, i.N*ld);		//0かi.N*ldの大きい方 を返す

		// specular color
		Vec3d lsp;		// 光源から反射する鏡面光の値
		/* lspの値を自分で計算しよう */
		Vec3d R = ld + 2 * ((i.N*ld)*i.N - ld);

		Vec3d V = -r.getDirection();	// 視線ベクトル
		double ns = shininess(i);
//		Vec3d hair = cuticleShade1(ld, i.N);
//		Vec3d hair = cuticleShade2(ld, i.N, V, color);
//		lsp = ks(i)*pow(max(0, R*V), ns);       //pow(a,b)  : aのb乗を返す
//		lsp = prod(ks(i),hair)*pow(max(0, R*V), ns);

		if (ks(i).n[0] == 0.0 && ks(i).n[1] == 0.0 && ks(i).n[2] == 0.0) {
			lsp = (0,0,0);
		}
		else {
			Vec3d hair = cuticleShade2(ld, i.N, V, color);
//			Vec3d hair = ellipseShade2(ld, i.N, V, color);
			lsp = prod(ks(i), hair)*pow(max(0, R*V), ns);
		}

		lum += prod(atten, prod(lc, ldf + lsp));
	}
	return lum;
	
//	return kd(i);

}

Vec3d Material::cuticleShade1(Vec3d L, Vec3d N) const
{
	string name = "Colordata1.txt";
	ifstream fp(name);
	if (!fp)	cout << "file error_" << name << endl;

	int in_max = 90;
	Vec3d color;
	int angle = acos((L*N) / (L.length() * N.length())) * 180 / PI;
	while (fp) {
		for (int i = 0; i < in_max; i++) {
			fp >> color.n[0] >> color.n[1] >> color.n[2];
			if (i == angle)	return color;
		}
	}

	return Vec3d(1,1,1);
}


Vec3d Material::cuticleShade2(Vec3d L, Vec3d N, Vec3d V) const
{
	string name = "Colordata2.txt";
	ifstream fp(name);
	if (!fp)	cout << "file error_" << name << endl;

	int i, j;
	Vec3d color;
	int in_d_deg = 5, out_d_deg = 1;
	int in_angle = acos((L*N) / (L.length() * N.length())) * 180 / PI;
	int out_angle = acos((V*N) / (V.length() * N.length())) * 180 / PI;
	while (fp) {
		fp >> color.n[0] >> color.n[1] >> color.n[2] >> i >> j;
		if ((int(i/in_d_deg) == int(in_angle/in_d_deg)) && (int(j/out_d_deg) == int(out_angle/out_d_deg)))	return color;
	}

	return Vec3d(1, 1, 1);
}

Vec3d Material::cuticleShade2(Vec3d L, Vec3d N, Vec3d V, Vec3d* c) const
{
	int in_d_deg = 1, out_d_deg = 1;
	int in_max = 180, out_max = 180;

	double r = 2;				// 球の半径
	Vec3d p = sqrt(r) * N;		// 注目点の位置ベクトル
	Vec3d W = Vec3d(-p.n[0], (r*r / p.n[1]) - p.n[1], -p.n[2]);		// 接線

	Vec3d V_ = (((W*W)*(N*V) - (N*W)*(W*V))*N + ((N*N)*(W*V) - (N*W)*(N*V))*W) / ((N*N)*(W*W) - (N*W)*(N*W));
	V = V_;			// 視線ベクトルを二次平面に射影したベクトル  適用しない場合はここをコメントアウト

	int in_angle = acos((L*N) / (L.length() * N.length())) * 180 / PI;
	int out_angle = acos((V*N) / (V.length() * N.length())) * 180 / PI;
	
	int lv_angle = acos((V*L) / (V.length() * L.length())) * 180 / PI;
	
	if (lv_angle - in_angle > 0 && lv_angle - out_angle > 0) {
		out_angle = 90 - out_angle;
	}
	else {
		out_angle += 90;
	}
	in_angle += 90;

	int inc = 5;
	in_angle -= inc;
	out_angle -= inc;

	if (in_angle > in_max)	in_angle = in_max;
	if (out_angle > out_max)	out_angle = out_max;

	int a = (out_max / out_d_deg) * (in_angle / in_d_deg) + (out_angle / out_d_deg);
	return c[a];
}

Vec3d Material::cuticleShade2_inf(Vec3d L, Vec3d N, Vec3d V, Vec3d* c) const
{														// カメラを無限遠方に仮定	とりあえず平面(plane.ray)に対応
	int in_d_deg = 1, out_d_deg = 1;
	int in_max = 180, out_max = 180;

	int in_angle = acos((L*N) / (L.length() * N.length())) * 180 / PI;
	int out_angle = -(90 - in_angle);

	in_angle += 90;
	out_angle += 90;

	int inc = 5;
	in_angle -= inc;
	out_angle -= inc;

	if (in_angle > in_max)	in_angle = in_max;
	if (out_angle > out_max)	out_angle = out_max;

	int a = (out_max / out_d_deg) * (in_angle / in_d_deg) + (out_angle / out_d_deg);
	return c[a];
}

Vec3d Material::ellipseShade2(Vec3d L, Vec3d N, Vec3d V, Vec3d* c) const
{
	int in_d_deg = 1, out_d_deg = 1;
	int in_max = 180, out_max = 180;

	double r = 2;				// 球の半径
	Vec3d p = sqrt(r) * N;		// 注目点の位置ベクトル
	Vec3d W = Vec3d(-p.n[0], (r*r / p.n[1]) - p.n[1], -p.n[2]);		// 接線
	Vec3d Z = W ^ N;	// WとNに垂直なベクトル = 外積

	Vec3d V_ = (((Z*Z)*(N*V) - (N*Z)*(Z*V))*N + ((N*N)*(Z*V) - (N*Z)*(N*V))*Z) / ((N*N)*(Z*Z) - (N*Z)*(N*Z));
	Vec3d L_ = (((Z*Z)*(N*L) - (N*Z)*(Z*L))*N + ((N*N)*(Z*L) - (N*Z)*(N*L))*Z) / ((N*N)*(Z*Z) - (N*Z)*(N*Z));
	V = V_;			// 視線ベクトルを二次平面に射影したベクトル  適用しない場合はここをコメントアウト
	L = L_;

	int in_angle = acos((L*N) / (L.length() * N.length())) * 180 / PI;
	int out_angle = acos((V*N) / (V.length() * N.length())) * 180 / PI;

	
	in_angle = 90 - in_angle;
	out_angle = 90 - out_angle;

	if (in_angle > in_max)		in_angle = in_max;
	if (out_angle > out_max)	out_angle = out_max; 
	if (in_angle < 0)			in_angle = 0;
	if (out_angle < 0)			out_angle = 0;

	int a = (out_max / out_d_deg) * (in_angle / in_d_deg) + (out_angle / out_d_deg);
	return c[a];
}


Vec3d* Material::cuticleShade2_set() const
{
	string name = "Colordata4_slab7.txt";
	ifstream fp(name);
	if (!fp)	cout << "file error_" << name << endl;

	int i, j, cell;
	Vec3d color;
	Vec3d out[33000];
	int in_d_deg = 1, out_d_deg = 1;
	int in_max = 180, out_max = 180;
	int a;
	while (fp) {
		fp >> color.n[0] >> color.n[1] >> color.n[2] >> i >> j >> cell;
		a = (out_max / out_d_deg) * (i / in_d_deg) + (j / out_d_deg);
		out[a] = color;
	}
	cout << "Hair_ColorData_set" << endl;
	return out;
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
	return getPixelAt(coord[0] * width, coord[1] * height);			//**** 追加箇所(テクスチャオプション) ****//
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

