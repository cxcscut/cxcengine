#ifndef CXC_PHYSICS_COLLIDER_H
#define CXC_PHYSICS_COLLIDER_H

#define MAX_CONTACT_NUM 64
#define DESITY 0.05f
#define dDOUBLE

#include <vector>
#include <memory>
#include "ode/ode.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Core/EngineCore.h"

namespace cxc {

	class RigidBody3D;

	enum class eCollisionChannel : uint16_t
	{
		CollisionFree = 0,
		WorldStatic = 1,
		WorldDynamic = 2
	};

	/* Collider3D is the base class for all the collider class */
	class CXC_ENGINECORE_API Collider3D : public std::enable_shared_from_this<Collider3D>
	{

	public:

		Collider3D();
		virtual ~Collider3D();

	public:
		
		void EnableGeom() noexcept;
		void DisableGeom() noexcept;
		bool isGeomEnable() const noexcept;

		void BindRigidBody(std::shared_ptr<RigidBody3D> pRigidBody) noexcept;

	public:

		void SetColliderRotation(const dMatrix3& RotMatrix);
		void SetColliderPosition(dReal x, dReal y, dReal z) noexcept;

		std::shared_ptr<RigidBody3D> GetOwnerRigidBody();
		dSpaceID GetGeomSpace() const noexcept;
		dGeomID GetColliderGeomID() const noexcept { return ColliderGeomID; };

	public:

		eCollisionChannel GetCollisionChannel() const { return CollisionChannel; }
		void SetCollisionChannel(eCollisionChannel Channel) { CollisionChannel = Channel; }

		virtual void CollisionCallback(std::shared_ptr<Collider3D> TargetCollider) {};

	protected:

		// Collision channel of the Collider
		eCollisionChannel CollisionChannel;

		// Weak pointer back to the rigidbody that bind to it
		std::weak_ptr<RigidBody3D> pOwnerRigidBody;

		// Collider gemo id
		dGeomID ColliderGeomID;
	};
}

#endif // CXC_PHYSICS_COLLIDER_H
