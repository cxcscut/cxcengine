#include "Material/Material.h"
#include "Material/MaterialManager.h"
#include "Material/Texture2D.h"
#include "Geometry/Mesh.h"

namespace cxc
{
	Material::Material()
	{
		OwnerObject.reset();

		// Default material is blue
		MaterialName = "DefaultMaterial";
		DiffuseFactor = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	Material::Material(const std::string& Name, const glm::vec3& Emissive, const glm::vec3& Ambient, const glm::vec3& Diffuse, const glm::vec3& Specular, float InShiniess)
	{
		OwnerObject.reset();

		MaterialName = Name;
		EmissiveFactor = Emissive;
		AmbientFactor = Ambient;
		DiffuseFactor = Diffuse;
		SpecularFactor = Specular;
		Shiniess = InShiniess;
	}

	Material::~Material()
	{

	}
}