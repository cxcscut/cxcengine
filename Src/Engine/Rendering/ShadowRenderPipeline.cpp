#include "Rendering/ShadowRenderPipeline.h"
#include "Rendering/ShadowRenderer.h"
#include "Rendering/RendererContext.h"
#include "World/World.h"
#include "Geometry/SubMesh.h"

namespace cxc
{
	ShadowRenderLightingPassPipeline::ShadowRenderLightingPassPipeline()
	{
		PipelineName = "LightingPassPipeline";
	}

	ShadowRenderLightingPassPipeline::~ShadowRenderLightingPassPipeline()
	{

	}

	void ShadowRenderLightingPassPipeline::BindLightUniforms(std::vector<std::shared_ptr<LightSource>> Lights, std::vector<GLuint>& SubroutineIndices)
	{
		if (Lights.empty())
			return;

		auto pLight = Lights[0];
		if (!pLight)
			return;

		GLint LightTargetPosLoc, SpotLightCutOffAngleLoc;
		GLint LightPosLoc, LightColorLoc, LightIntensityLoc, LightAttenuationLoc;
		GLint LightAttenuationSubroutineIndex;
		std::string LightUniformNamePrefix, LightAttenuationSubroutineName;

		switch (pLight->GetLightType())
		{
		case eLightType::OmniDirectional:
		{
			LightUniformNamePrefix = "OmniLight";
			LightAttenuationSubroutineName = "OmniLightAttenuation";
			break;
		}
		case eLightType::Directional:
		{
			LightUniformNamePrefix = "DirectionalLight";
			LightAttenuationSubroutineName = "DirectionalLightAtteunation";
			break;
		}
		case eLightType::Spot:
		{
			LightUniformNamePrefix = "SpotLight";
			LightAttenuationSubroutineName = "SpotLightAtteunations";
			break;
		}
		}

		SpotLightCutOffAngleLoc = glGetUniformLocation(ProgramID, (LightUniformNamePrefix + ".CutOffAngle").c_str());
		LightTargetPosLoc = glGetUniformLocation(ProgramID, (LightUniformNamePrefix + ".TargetPos").c_str());
		LightPosLoc = glGetUniformLocation(ProgramID, (LightUniformNamePrefix + ".Position").c_str());
		LightColorLoc = glGetUniformLocation(ProgramID, (LightUniformNamePrefix + ".Color").c_str());
		LightIntensityLoc = glGetUniformLocation(ProgramID, (LightUniformNamePrefix + ".Intensity").c_str());
		LightAttenuationLoc = glGetSubroutineUniformLocation(ProgramID, GL_FRAGMENT_SHADER, (LightAttenuationSubroutineName).c_str());

		switch (pLight->GetAtteunationType())
		{
		case eLightAtteunationType::None:
			LightAttenuationSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "None");
			break;

		case eLightAtteunationType::Linear:
			LightAttenuationSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "Linear");
			break;

		case eLightAtteunationType::Quadratic:
			LightAttenuationSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "Quadratic");
			break;

		case eLightAtteunationType::Cubic:
			LightAttenuationSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "Cubic");
			break;
		}

		if (LightAttenuationLoc >= 0)
		{
			SubroutineIndices[LightAttenuationLoc] = LightAttenuationSubroutineIndex;
		}

		// Light properties
		glUniform1f(SpotLightCutOffAngleLoc, glm::radians(pLight->GetCutOffAngle()));
		glUniform3f(LightTargetPosLoc, pLight->GetTargetPos()[0], pLight->GetTargetPos()[1], pLight->GetTargetPos()[2]);
		glUniform3f(LightPosLoc, pLight->GetLightPos()[0], pLight->GetLightPos()[1], pLight->GetLightPos()[2]);
		glUniform3f(LightColorLoc, pLight->GetLightColor()[0], pLight->GetLightColor()[1], pLight->GetLightColor()[2]);
		glUniform1f(LightIntensityLoc, pLight->GetIntensity());
	}

	void ShadowRenderLightingPassPipeline::Render(std::shared_ptr<RendererContext> Context)
	{
		GLint depthBiasMVP_loc;
		GLint ShadowMapSampler_loc, Eyepos_loc, M_MatrixID;
		GLint shadowmapCube_loc;
		GLint LightPowerLoc;

		auto pSceneManager = SceneManager::GetInstance();
		auto Lights = Context->GetLightsArray();
		if (Lights.empty())
			return;

		auto pLight = Lights[0];
		if (!pLight)
			return;

		auto pWorld = World::GetInstance();
		auto pShadowRenderer = std::dynamic_pointer_cast<ShadowRenderer>(pOwnerRenderer.lock());
		if (!pShadowRenderer)
			return;

		auto BindedSubMeshes = Context->GetBindedSubMeshes();

		for (auto pSubMesh : BindedSubMeshes)
		{
			auto pOwnerMesh = pSubMesh->GetOwnerMesh();

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, pWorld->pWindowMgr->GetWindowWidth(), pWorld->pWindowMgr->GetWindowHeight());

			Eyepos_loc = glGetUniformLocation(ProgramID, "EyePosition_worldspace");
			M_MatrixID = glGetUniformLocation(ProgramID, "M");
			LightPowerLoc = glGetUniformLocation(ProgramID, "LightPower");

			shadowmapCube_loc = glGetUniformLocation(ProgramID, "shadowmapCube");
			depthBiasMVP_loc = glGetUniformLocation(ProgramID, "DepthBiasMVP");
			ShadowMapSampler_loc = glGetUniformLocation(ProgramID, "shadowmap");

			// Get subroutine uniforms info
			GLsizei ActiveSubroutinesUniformCountFS;
			glGetProgramStageiv(ProgramID, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &ActiveSubroutinesUniformCountFS);
			std::vector<GLuint> SubroutineIndicesFS(ActiveSubroutinesUniformCountFS, 0);

			// Bind depth texture to the texture unit 1
			// We use texture unit 0 for the objects texture sampling 
			// while texture unit 1 for depth buffer sampling
			glActiveTexture(GL_TEXTURE0 + (GLuint)TextureUnit::ShadowTextureUnit);
			if (pLight->GetLightType() == eLightType::OmniDirectional)
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, pShadowRenderer->GetShadowCubeMap());
				glUniform1i(shadowmapCube_loc, (GLuint)TextureUnit::ShadowTextureUnit);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, pShadowRenderer->GetShadowMapDepthTexture());
				glUniform1i(ShadowMapSampler_loc, (GLuint)TextureUnit::ShadowTextureUnit);
			}

			// the bias for depthMVP to map the NDC coordinate from [-1,1] to [0,1] which is a necessity for texture sampling
			glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
			);

			glm::mat4 depthBiasMVP = biasMatrix * pShadowRenderer->GetShadowMapDepthVP() * pOwnerMesh->GetModelMatrix();
			glUniformMatrix4fv(depthBiasMVP_loc, 1, GL_FALSE, &depthBiasMVP[0][0]);

			auto CurrentActiveCamera = pWorld->pSceneMgr->GetCurrentActiveCamera();
			if (CurrentActiveCamera)
			{
				glm::vec3 EyePosition = CurrentActiveCamera->EyePosition;
				glUniform3f(Eyepos_loc, EyePosition.x, EyePosition.y, EyePosition.z);
			}

			BindLightUniforms(Lights, SubroutineIndicesFS);

			// Set model matrix
			glUniformMatrix4fv(M_MatrixID, 1, GL_FALSE, &pOwnerMesh->GetModelMatrix()[0][0]);

			// Bind the material of the mesh
			MaterialDiffuseSubroutineInfo DiffuseModelInfo;
			DiffuseModelInfo.SubroutineUniformName = "DiffuseFactorSelection";
			DiffuseModelInfo.TexturedSubroutineName = "TextureDiffuse";
			DiffuseModelInfo.NonTexturedSubroutineName = "NonTextureDiffuse";
			pSubMesh->BindMaterial(ProgramID, DiffuseModelInfo, SubroutineIndicesFS);

			// Set the light type selection subroutine
			GLint LightTypeSubroutineLoc = glGetSubroutineUniformLocation(ProgramID, GL_FRAGMENT_SHADER, "LightTypeSelection");
			GLuint LightTypeSubroutineIndex;
			switch (pLight->GetLightType())
			{
			case eLightType::Directional:
				LightTypeSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "DirectionalLightFragShading");
				break;
			case eLightType::OmniDirectional:
				LightTypeSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "OmniLightFragShading");
				break;
			case eLightType::Spot:
				LightTypeSubroutineIndex = glGetSubroutineIndex(ProgramID, GL_FRAGMENT_SHADER, "SpotLightFragShading");
				break;
			}

			if (LightTypeSubroutineLoc >= 0)
				SubroutineIndicesFS[LightTypeSubroutineLoc] = LightTypeSubroutineIndex;

			// Submit the subroutines selections
			if (ActiveSubroutinesUniformCountFS > 0)
				glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, SubroutineIndicesFS.size(), &SubroutineIndicesFS.front());

			// Draw the mesh
			pSubMesh->DrawSubMesh();
		}
	}

	ShadowRenderBasePassPipeline::ShadowRenderBasePassPipeline()
		: SubMeshRenderPipeline()
	{
		PipelineName = "BasePassPipeline";
	}

	ShadowRenderBasePassPipeline::~ShadowRenderBasePassPipeline()
	{

	}

	void ShadowRenderBasePassPipeline::RenderShadowsToTexture(std::shared_ptr<RendererContext> Context, const std::vector<std::shared_ptr<LightSource>>& Lights)
	{
		auto pRenderer = pOwnerRenderer.lock();
		std::shared_ptr<ShadowRenderer> pShadowRenderer = std::dynamic_pointer_cast<ShadowRenderer>(pRenderer);
		if (!pShadowRenderer)
			return;

		auto BindedSubMeshes = Context->GetBindedSubMeshes();
		for (auto pSubMesh : BindedSubMeshes)
		{
			auto pOwnerMesh = pSubMesh->GetOwnerMesh();
			glViewport(0, 0, pShadowRenderer->GetShadowMapSize(), pShadowRenderer->GetShadowMapSize());

			GLint depthMVP_Loc = glGetUniformLocation(ProgramID, "depthMVP");
			glm::mat4 depthMVP;

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glBindVertexArray(pOwnerMesh->GetVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pSubMesh->GetSubMeshEBO());

			glBindBuffer(GL_ARRAY_BUFFER, pOwnerMesh->GetVertexCoordsVBO());
			glEnableVertexAttribArray(static_cast<GLuint>(Location::VERTEX_LOCATION));
			glVertexAttribPointer(static_cast<GLuint>(Location::VERTEX_LOCATION), 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); // Vertex position

			// Rendering depth map of the mesh to depth texture
			depthMVP = pShadowRenderer->GetShadowMapDepthVP() * pOwnerMesh->GetModelMatrix();

			glUniformMatrix4fv(depthMVP_Loc, 1, GL_FALSE, &depthMVP[0][0]);

			// Draw mesh depth to the texture
			pSubMesh->DrawSubMesh();
		}
	}

	void ShadowRenderBasePassPipeline::CookShadowMapDepthTexture(std::shared_ptr<RendererContext> Context, const std::vector<std::shared_ptr<LightSource>>& Lights)
	{
		auto pRenderer = pOwnerRenderer.lock();
		std::shared_ptr<ShadowRenderer> pShadowRenderer = std::dynamic_pointer_cast<ShadowRenderer>(pRenderer);
		if (!pShadowRenderer)
			return;

		auto pLight = Lights[0];
		if (!pLight)
			return;

		auto CubeMapIterator = pShadowRenderer->GetCubeMapPose();

		glBindFramebuffer(GL_FRAMEBUFFER, pShadowRenderer->GetShadowMapFBO());
		glViewport(0, 0, pShadowRenderer->GetShadowMapSize(), pShadowRenderer->GetShadowMapSize());
		if (pLight->GetLightType() == eLightType::OmniDirectional)
		{
			// Draw 6 faces of cube map
			for (uint16_t k = 0; k < 6; k++)
			{
				// Draw shadow of one face into the cube map 
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CubeMapIterator[k].CubeMapFace, pShadowRenderer->GetShadowCubeMap(), 0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, pShadowRenderer->GetShadowCubeMap());

				// Set the depth matrix correspondingly, FOV of the projection matrix must be 90 degrees to capture the whole scene
				pShadowRenderer->SetLightSpaceMatrix(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10000.0f),
					glm::lookAt(pLight->GetLightPos(), pLight->GetLightPos() + CubeMapIterator[k].Direction, CubeMapIterator[k].UpVector));

				// Render the shadows to the CubeMap
				RenderShadowsToTexture(Context, Lights);
			}
		}
		else
		{
			// Render the shadows to the Shadowmap
			RenderShadowsToTexture(Context, Lights);
		}
	}

	void ShadowRenderBasePassPipeline::Render(std::shared_ptr<RendererContext> Context)
	{
		auto pSceneManager = SceneManager::GetInstance();
		auto Lights = pSceneManager->GetLightsArray();

		// Cook shadow depth texture
		CookShadowMapDepthTexture(Context, Lights);
	}
}