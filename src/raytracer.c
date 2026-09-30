#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <math.h>

typedef enum{
	SphereObj,
	TriangleObj
}ObjectType;

typedef struct {
  float x;
  float y;
  float z;
}Vec3;

typedef struct {
	Vec3 color;
	int reflective;
}Material;

typedef struct {
  Vec3 origin;
  Vec3 direction;
  Vec3 normalized;
}Ray;

typedef struct {
	Vec3 position;
	Material material;
	ObjectType objType;
	float t;
	int bounces;
}RayHit;

typedef struct {
	Vec3 a;
	Vec3 b;
	Vec3 c;
	Vec3 normal;
	Material material;
}Triangle;

typedef struct {
	Vec3 position;
	float radius;
	Material material;
}Sphere;	

// Global vars
Material refl = { .color = {0,0,0}, .reflective = 1 };
Material blue = { .color = {0,0,255}, .reflective = 0 };
Material red =  { .color = {255,0,0}, .reflective = 0 };
Material white = { .color = {255,255,255}, .reflective = 0 };

Vec3 light = {3,5,-15};
float diffusionThres = 0.2;
int rayBouncesThres = 10;

Sphere spheres[100];
int numSpheres = 0;

Triangle triangles[100];
int numTriangles=0;

void colorPixel(unsigned char *array, int index, Vec3 color) {
  array[index]     = color.x; // Red
  array[index + 1] = color.y; // Green
  array[index + 2] = color.z; // Blue
}

void normalize(Ray* ray){
	// norm
	float norm = sqrt((pow(ray->direction.x, 2)
										 + pow(ray->direction.y, 2)
										 + pow(ray->direction.z, 2)));

	// normalize = v / ||v||
	ray->normalized.x = ray->direction.x/norm;
	ray->normalized.y = ray->direction.y/norm;
	ray->normalized.z = ray->direction.z/norm;
}

Ray getSurfaceNormal(RayHit rayHit, Vec3 objPosition){
	Ray surfaceNormal = {
		.direction.x = rayHit.position.x - objPosition.x,
		.direction.y = rayHit.position.y - objPosition.y,
		.direction.z = rayHit.position.z - objPosition.z
	};

	normalize(&surfaceNormal);

	return surfaceNormal;
}

Vec3 getCalcReflection(Vec3 d, Vec3 n){
	float dDotN = (d.x * n.x) + (d.y * n.y) + (d.z * n.z);
	Vec3 nDDotN = {(-2 * dDotN) * n.x,
								 (-2 * dDotN) * n.y,
								 (-2 * dDotN) * n.z};
	Vec3 added = {d.x + nDDotN.x,
								d.y + nDDotN.y,
								d.z + nDDotN.z};
	return added;
}

void getNormal(Triangle* obj){
	Vec3 temp1;
	temp1.x = obj->b.x - obj->a.x;
	temp1.y = obj->b.y - obj->a.y;
	temp1.z = obj->b.z - obj->a.z;

	Vec3 temp2;
	temp2.x = obj->c.x - obj->a.x;
	temp2.y = obj->c.y - obj->a.y;
	temp2.z = obj->c.z - obj->a.z;

	
	Vec3 final = {temp1.y * temp2.z - temp1.z * temp2.y,
		            temp1.z * temp2.x - temp1.x * temp2.z,
								temp1.x * temp2.y - temp1.y * temp2.x};

	obj->normal = final;
}

RayHit getRayDistanceTriangle(Ray* ray, Triangle* obj) {
	RayHit rayHit;
	float a = obj->a.x - obj->b.x;
	float b = obj->a.y - obj->b.y;
	float c = obj->a.z - obj->b.z;
	float d = obj->a.x - obj->c.x;
	float e = obj->a.y - obj->c.y;
	float f = obj->a.z - obj->c.z;
	float g = ray->normalized.x;
	float h = ray->normalized.y;
	float i = ray->normalized.z;
	float j = obj->a.x - ray->origin.x; 
	float k = obj->a.y - ray->origin.y;
	float l = obj->a.z - ray->origin.z;

	float m = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);

	float t = (-(f*(a*k - j*b) + e*(j*c - a*l) + d*(b*l - k*c)))/m;
	if (t < 0 ){//|| t > closestHit){
		rayHit.t = -1;
		return rayHit;
	}
	float gamma = (j*(e*i - h*f) + k*(g*f - d*i) + l*(d*h - e*g))/m;
	if (gamma < 0 || gamma > 1){
		rayHit.t = -1;
		return rayHit;
	}
	float beta = (i*(a*k - j*b) + h*(j*c - a*l) + g*(b*l - k*c))/m;
	if ( beta < 0 || beta > 1-gamma){
		rayHit.t = -1;
		return rayHit;
	}
	// hit triangle or triangle edge
	rayHit.t = t;
	rayHit.position.x = ray->origin.x + t * ray->normalized.x;
	rayHit.position.y = ray->origin.y + t * ray->normalized.y;
	rayHit.position.z = ray->origin.z + t * ray->normalized.z;
	rayHit.material = obj->material;
	rayHit.objType = TriangleObj;
	return rayHit;
}

RayHit getRayDistanceSphere(Ray* ray, Sphere* obj) {
	Vec3 d = ray->normalized;
	Vec3 e = ray->origin;
	Vec3 c = obj->position;
	float R = obj->radius;

	RayHit rayHit;

	Vec3 eMinusC;
	eMinusC.x = e.x - c.x;
	eMinusC.y = e.y - c.y;
	eMinusC.z = e.z - c.z;

	float dDotD =
    d.x * d.x +
    d.y * d.y +
    d.z * d.z;

  
	float dDotEMinusC =
    d.x * eMinusC.x +
    d.y * eMinusC.y +
    d.z * eMinusC.z;

	float eMinusCdotProduct =
    eMinusC.x * eMinusC.x +
    eMinusC.y * eMinusC.y +
    eMinusC.z * eMinusC.z;

	float discrim =
    (dDotEMinusC * dDotEMinusC) -
    (dDotD * (eMinusCdotProduct - R * R));

	if (discrim < 0) {
    rayHit.t = -1;
    return rayHit;
	}

	float sqrtDiscrim = sqrt(discrim);

	float negDDotEMinusC = -dDotEMinusC;

	float t1 = (negDDotEMinusC + sqrtDiscrim) / dDotD;
	float t2 = (negDDotEMinusC - sqrtDiscrim) / dDotD;
	float t;
	float thres = 0.001;

	// ignore small t values to stop acne
	if (t1 > thres && t2 > thres) {
    if (t1 < t2) {
			t = t1;
    } else {
			t = t2;
    }
	}
	else if (t1 > thres) {
    t = t1;
	}
	else if (t2 > thres) {
    t = t2;
	}
	else {
		t = -1;
	}

	rayHit.t = t;
	rayHit.position.x = ray->origin.x + t * ray->normalized.x;
	rayHit.position.y = ray->origin.y + t * ray->normalized.y;
	rayHit.position.z = ray->origin.z + t * ray->normalized.z;
	rayHit.material = obj->material;
	rayHit.objType = SphereObj;
	return rayHit;
}

float diffuseSphere(RayHit rayHit, Vec3 objPosition){
	Ray surfaceNormal = getSurfaceNormal(rayHit, objPosition);


	Ray lightRay = {.direction.x = light.x-rayHit.position.x,
									.direction.y = light.y-rayHit.position.y,
									.direction.z = light.z-rayHit.position.z};
	normalize(&lightRay);
	float diffusion = ((surfaceNormal.normalized.x * lightRay.normalized.x)
										 + (surfaceNormal.normalized.y * lightRay.normalized.y)
										 + (surfaceNormal.normalized.z * lightRay.normalized.z));

	if (diffusion < diffusionThres){
		diffusion = diffusionThres;
	}
	return diffusion;
}

float diffuseTriangle(RayHit rayHit, Triangle triangle) {
    Vec3 normal = triangle.normal;

    // Normalize triangle normal
    float length = sqrt(
        normal.x * normal.x +
        normal.y * normal.y +
        normal.z * normal.z
    );

    normal.x /= length;
    normal.y /= length;
    normal.z /= length;

    // Direction from surface to light
    Vec3 lightDirection = {
        light.x - rayHit.position.x,
        light.y - rayHit.position.y,
        light.z - rayHit.position.z
    };

    float lightLength = sqrt(
        lightDirection.x * lightDirection.x +
        lightDirection.y * lightDirection.y +
        lightDirection.z * lightDirection.z
    );

    lightDirection.x /= lightLength;
    lightDirection.y /= lightLength;
    lightDirection.z /= lightLength;

    float diffusion =
        normal.x * lightDirection.x +
        normal.y * lightDirection.y +
        normal.z * lightDirection.z;

    if (diffusion < diffusionThres) {
        diffusion = diffusionThres;
    }

    return diffusion;
}


void getClosestSphere(Ray* ray, RayHit* closestRayHit, Sphere* closestSphere){
	for(int i=0; i<numSpheres; i++){
		Sphere circle = spheres[i];
		// Get rayHit data
		RayHit rayHit = getRayDistanceSphere(ray, &circle);
		// If we hit something
		if (rayHit.t > -1){
			// if we didn't hit anything assign what we hit
			if(closestRayHit->t < 0 || rayHit.t < closestRayHit->t){
				*closestRayHit = rayHit;
				*closestSphere = circle;
			}
		}
	}
}

void getClosestTriangle(Ray* ray, RayHit* closestRayHit, Triangle* closestTriangle){
	for(int i=0; i<numTriangles; i++){
		Triangle triangle = triangles[i];
		// Get rayHit data
		RayHit rayHit = getRayDistanceTriangle(ray, &triangle);
		// If we hit something
		if (rayHit.t > -1){
			// if we didn't hit anything assign what we hit
			if(closestRayHit->t < 0 || rayHit.t < closestRayHit->t){
				*closestRayHit = rayHit;
				*closestTriangle = triangle;
			}
		}
	}
}

int inShadow(Ray ray, RayHit rayHit) { // Shoot ray and if something else other than light is hit return 1
	// Direction from surface to light
	Ray lightRay;
	lightRay.origin = (Vec3) {
		rayHit.position.x,
		rayHit.position.y,
		rayHit.position.z
	};

	lightRay.direction = (Vec3) {
		light.x - rayHit.position.x,
		light.y - rayHit.position.y,
		light.z - rayHit.position.z
	};

	// Check if something is inbetween light and source
	// Get closest Sphere
	RayHit closestRayHit;
	closestRayHit.t = -1; // assume no hit
	Sphere closestSphere = spheres[0];
	closestRayHit = getRayDistanceSphere(&lightRay, &closestSphere);
	closestRayHit.bounces = 0;
	getClosestSphere(&lightRay, &closestRayHit, &closestSphere);

	// Check closest rayhit of triangles 
	Triangle closestTriangle = triangles[0];
	getClosestTriangle(&lightRay, &closestRayHit, &closestTriangle);

	if (closestRayHit.t < 0){
		return 0;
	} 

	return 1;
}


int main() {
  char filename[] = "reference.png";
  int width = 512;
  int height = 512;
  float pixelWidth = 2/(float)width;
  unsigned char *arrayContainingImage;
  int arrayContainingImageSize = width * height * 3;
  arrayContainingImage = malloc(arrayContainingImageSize);
	// Create objects
	spheres[0] = (Sphere) { .position = { 0,0,-16 }, .radius = 2, .material = refl };
	spheres[1] = (Sphere) { .position = { 3,-1,-14 }, .radius = 1, .material = refl};
	spheres[2] = (Sphere) { .position = { -3,-1,-14 }, .radius = 1, .material = red };
	numSpheres = 3;
	// back wall
	triangles[0] = (Triangle) { .a = { -8,-2,-20 }, .b = {8,-2,-20}, .c = {8,10,-20}, .material = blue };
	triangles[1] = (Triangle) { .a = { -8,-2,-20 }, .b = {8,10,-20}, .c = {-8,10,-20}, .material = blue };
	// floor
	triangles[2] = (Triangle) { .a = { -8,-2,-20 }, .b = {8,-2,-10}, .c = {8,-2,-20}, .material = white };
	triangles[3] = (Triangle) { .a = { -8,-2,-20 }, .b = {-8,-2,-10}, .c =  {8,-2,-10}, .material = white };
	// right red triangle
	triangles[4] = (Triangle) { .a = { 8,-2,-20 }, .b = {8,-2,-10}, .c = {8,10,-20}, .material = red };
	numTriangles = 5;

  float y3D = 1;
  float x3D = -1;
  if (arrayContainingImage == NULL) {
    printf("ERROR: allocating failed");
  }

  // Create camera rays
  Ray ray;
  ray.origin.x = 0;
  ray.origin.y = 0;
  ray.origin.z = 0;
  ray.direction.z = -2;

  // Color array
  for (int y = 0; y < height; y++) {
    x3D = -1; // reset after each row
    for (int x = 0; x < width; x++) {
      int index = (y * width + x) * 3;
      // Convert xy to Ray vector
      ray.direction.x = x3D+(pixelWidth/2); // go to center of pixel
      ray.direction.y = y3D-(pixelWidth/2);

      // Normalizing
			normalize(&ray);

			// Get closest Sphere
			RayHit closestRayHit;
			closestRayHit.t = -1; // assume no hit
			Sphere closestSphere = spheres[0];
			closestRayHit = getRayDistanceSphere(&ray, &closestSphere);
			closestRayHit.bounces = 0;
			getClosestSphere(&ray, &closestRayHit, &closestSphere);

			// Check closest rayhit of triangles 
			Triangle closestTriangle = triangles[0];
			getClosestTriangle(&ray, &closestRayHit, &closestTriangle);
			// Check for reflections and diffusion
			if (closestRayHit.t >= 0){ // We only hit stuff that is infront of us.
			switch (closestRayHit.objType){
			case SphereObj:
				if (closestSphere.material.reflective){
					Vec3 color;
					float diffusion = 1.0;
					Ray reflectionRay = ray;
					// Bounce ray again until 10 bounces then color black
					while(closestRayHit.bounces < rayBouncesThres){
						closestRayHit.bounces += 1;

						Ray reflectionSurfaceNormal = getSurfaceNormal(closestRayHit,
																													 closestSphere.position);
						reflectionRay.direction = getCalcReflection(reflectionRay.normalized, 
																												reflectionSurfaceNormal.normalized);

						// Normalize and update new origin to where old ray hit
						normalize(&reflectionRay);
						reflectionRay.origin = closestRayHit.position;

						// get new rayhit and closest obj
						closestRayHit.t = -1; // assume no hit
						getClosestSphere(&reflectionRay, &closestRayHit, &closestSphere);
						getClosestTriangle(&reflectionRay, &closestRayHit, &closestTriangle);

						if (closestRayHit.t < 0){ // Didn't hit anything, color black
							color = (Vec3) {0,0,0};
							break;
						}else if (!closestRayHit.material.reflective){
							if (closestRayHit.objType == SphereObj){
								diffusion = diffuseSphere(closestRayHit, closestSphere.position);
								break;
							}else if(closestRayHit.objType == TriangleObj){ // not drawing triangle bc 'nan' value returned
								getNormal(&closestTriangle);
								diffusion = diffuseTriangle(closestRayHit, closestTriangle);
								if (inShadow(ray, closestRayHit)){
									diffusion = diffusionThres;
								}
									
								break;
							}
						}
					}

					color = (Vec3){closestRayHit.material.color.x * diffusion,
												closestRayHit.material.color.y * diffusion,
												closestRayHit.material.color.z * diffusion};

					colorPixel(arrayContainingImage, index, color);
				}else{
					float diffusion = diffuseSphere(closestRayHit, closestSphere.position);
					if (inShadow(ray, closestRayHit)){
						diffusion = diffusionThres;
					}
					Vec3 color = {closestRayHit.material.color.x * diffusion,
												closestRayHit.material.color.y * diffusion,
												closestRayHit.material.color.z * diffusion};

					colorPixel(arrayContainingImage, index, color);
				}
			break;
			case TriangleObj:
				// Get diffussion value via RayHit obj
				getNormal(&closestTriangle);
				if (closestRayHit.t >= 0){ // We only hit stuff that is infront of us.
					if (closestTriangle.material.reflective){
						// Bounce ray again until 10 bounces then color black-------------------------------------------------------------
					}else{ 
						float diffusion = diffuseTriangle(closestRayHit, closestTriangle);
						if (inShadow(ray, closestRayHit)){
							diffusion = diffusionThres;
						}
						Vec3 color = {closestRayHit.material.color.x * diffusion,
													closestRayHit.material.color.y * diffusion,
													closestRayHit.material.color.z * diffusion};

						colorPixel(arrayContainingImage, index, color);
					}
				}
				break;
			}
		}

      x3D += (pixelWidth);
    }
    // Move pixel
    y3D -= (pixelWidth);
  }

  // Send array in for processing
  stbi_write_png(filename, width, height, 3, arrayContainingImage, width * 3);
  free(arrayContainingImage);
}
