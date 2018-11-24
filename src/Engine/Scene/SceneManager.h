
#ifdef WIN32

#include "..\General\DefineTypes.h"
#include "..\Utilities\Singleton.inl"
#include "..\Scene\Object3D.h"
#include "..\Material\MaterialManager.h"
#include "..\Scene\Camera.h"
#include "..\Utilities\FBXSDKUtility.h"
#include "..\Rendering\RenderManager.h"

#else

#include "../General/DefineTypes.h"
#include "../Utilities/Singleton.inl"
#include "../Scene/Object3D.h"
#include "../Material/MaterialManager.h"
#include "../Scene/Camera.h"
#include "../Utilities/FBXSDKUtility.h"
#include "../Rendering/RenderManager.h"

#endif // WIN32

#ifndef CXC_SCENEMANAGER_H
#define CXC_SCENEMANAGER_H

#define PARTITION_MIN_NODENUM 8
#define MAX_PARTITION_DEPTH 8
#define MOVE_NORTH 1
#define MOVE_SOUTH 2
#define MOVE_EAST 3
#define MOVE_WEST 4
#define MOVE_UP 5
#define MOVE_DOWN 6

#define WOLRD_QUICK_STEPSIZE 0.02f

namespace cxc {

	class OctreeRoot;

	class OctreeNode {

	public:

		explicit OctreeNode(const CXCRect3 &SceneSize);

		virtual ~OctreeNode();
		
	public:

		bool InsertObject(std::shared_ptr<Object3D> pObject) noexcept;

		// Find all the objects that within the bounding box
		void FindObjects(const CXCRect3 &_AABB, std::vector<std::shared_ptr<Object3D>> &objects ) noexcept;
		void SpacePartition() noexcept;
		void SpaceMerge() noexcept;

		// Find the nodes that contains the give object
		bool FindNode(std::shared_ptr<Object3D> pTarget, std::vector<OctreeNode*> &nodes) noexcept;

		static bool isNorth(char ch);
		static bool isSouth(char ch);
		static bool isEast(char ch);
		static bool isWest(char ch);
		static bool isUp(char ch);
		static bool isDown(char ch);

		bool isBoundaryNode() const noexcept;

		std::string GetNorthCode() const noexcept;
		std::string GetSouthCode() const noexcept;
		std::string GetEastCode() const noexcept;
		std::string GetWestCode() const noexcept;
		std::string GetUpCode() const noexcept;
		std::string GetDownCode() const noexcept;
		int GetObjNum() const noexcept;

		void MoveAndAdjust(const std::string &name,int direction);

	public:

		OctreeRoot* m_rootPtr;
		OctreeNode* parent;

		// Objects of the Node
		std::unordered_map<std::string,std::shared_ptr<Object3D>> Objects;

		// AABB bounding box of the node
		CXCRect3 AABB;

		bool isLeaf;

		// Code of the node
		std::string code;
	};

	class OctreeRoot : public OctreeNode
	{
	public:

		OctreeRoot(const glm::vec3 &center, const CXCRect3 &size);
		~OctreeRoot();

	public:

		std::shared_ptr<OctreeNode> FindNode(const std::string &code) noexcept;
		std::shared_ptr<OctreeNode> CreateSpace(const std::string &code, const CXCRect3 &AABB) noexcept;;
		void RemoveNode(const std::string &name) noexcept;

	private:

		std::unordered_map<std::string, std::shared_ptr<OctreeNode>> Nodes;

	};

	class SceneManager final : public Singleton<SceneManager>
	{

	public:

		friend Singleton<SceneManager>;
		friend class World;

		explicit SceneManager();
		~SceneManager();

		SceneManager(const SceneManager&) = delete;
		SceneManager(const SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&&) = delete;

	// Scene loading from fbx file via FBX SDKs
	public :

		bool LoadSceneFromFBX(const std::string& filepath) noexcept;
		void ProcessSceneNode(FbxNode* pNode) noexcept;

	// Object loading from obj files
	public:

		// Delete object
		void DeleteObject(const std::string &sprite_name) noexcept;

	public:

		// Pointer to resource manager
		std::shared_ptr<MaterialManager> pMaterialMgr;
		std::shared_ptr<Camera> pCamera;
		std::shared_ptr<RenderManager> pRenderMgr;

		// Return the pointer to the specific sprite
		std::shared_ptr<Object3D > GetObject3D (const std::string &sprite_name) const noexcept;

		// Camera interface
		void InitCameraStatus(GLFWwindow * window) noexcept;
		void SetCameraParams(const glm::vec3 &eye, const glm::vec3 &origin, const glm::vec3 &up,
							const glm::mat4 &ProjectionMatrix) noexcept;
		void UpdateCameraPos(GLFWwindow *window, float x, float y, GLuint height, GLuint width) noexcept;
		void SetCameraMode(CameraModeType mode) noexcept;

		// Return the SpriteMap
		const std::unordered_map<std::string, std::shared_ptr<Object3D >> &GetObjectMap() const noexcept;

		void SetCenter(const glm::vec3 &center) noexcept { m_SceneCenter = center; };
		void SetSize(float size) noexcept { m_SceneSize = size; };

		void AddLight(const std::string& Name, const glm::vec3& LightPosition, const glm::vec3& LightDirection, eLightType Type);
		void RemoveLight(const std::string& LightName);
		std::shared_ptr<BaseLighting> GetLight(uint32_t LightIndex);
		std::shared_ptr<BaseLighting> GetLight(const std::string& LightName);
		uint32_t GetLightCount() const { return Lights.size(); }

	public:

		void Tick(float DeltaSeconds) noexcept;

		void RenderScene() noexcept;

	private:

		void AllocateBuffers();
		void ReleaseBuffers();

		void PreRender(const std::vector<std::shared_ptr<BaseLighting>>& Lights) noexcept;
		void Render(const std::vector<std::shared_ptr<BaseLighting>>& Lights) noexcept;
		void PostRender(const std::vector<std::shared_ptr<BaseLighting>>& Lights) noexcept;

	// Physics settings
	public:

		void BuildOctree() noexcept;
		void UpdateMeshTransMatrix() noexcept;

	private:

		void UpdateBoundary(const CXCRect3 &AABB) noexcept;

		// Add object to object map
		void AddObjectInternal(const std::string &SpriteName, const std::shared_ptr<Object3D> &ObjectPtr, bool isKinematics = false) noexcept;

	private:

		// Lights
		std::vector<std::shared_ptr<BaseLighting>> Lights;

		// Hashmap for culling
		std::unordered_set<std::shared_ptr<Object3D>> hash;

		// <Object Name , Pointer to object>
		std::unordered_map<std::string, std::shared_ptr<Object3D>> m_ObjectMap;

		glm::vec3 m_SceneCenter;

		float m_SceneSize;

		// Space partition
		std::shared_ptr<OctreeRoot> pRoot;

		CXCRect3 m_Boundary;
	};

}
#endif // CXC_SCENEMANAGER_H