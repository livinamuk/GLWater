#include "GL_renderer.h"
#include "GL_backend.h"
#include "Types/GL_detachedMesh.hpp"
#include "Types/GL_frameBuffer.hpp"
#include "Types/GL_shader.h"
#include "../Managers/AssetManager.h"
#include "../Camera.h"
#include "../Scene.hpp"

namespace OpenGLRenderer {

    Shader g_solidColorShader;
    Shader gTexturedShader;
    Shader gWaterShader;
    Shader gHairMaskShader;

    ComputeShader gComputeTest;

    GLFrameBuffer gFrameBuffer;
    GLFrameBuffer gWaterReflectionFrameBuffer;
    GLFrameBuffer gWaterRefractionFrameBuffer;

    OpenGLDetachedMesh g_debugLinesMesh;
    OpenGLDetachedMesh g_debugPointsMesh;

    std::vector<Vertex> g_debugLines;
    std::vector<Vertex> g_debugPoints;

    void DrawScene(Shader& shader, bool renderHair);
    void RenderLighting();
    void RenderHair();
    void RenderWater();
    void RenderDebug();

    void Init() {
        float scale = 3;
        gFrameBuffer.Create("Main", 640 * scale, 360 * scale);
        gFrameBuffer.CreateAttachment("Color", GL_RGBA8);
        gFrameBuffer.CreateAttachment("Position", GL_RGBA16F);
        gFrameBuffer.CreateAttachment("UnderWaterColor", GL_RGBA8);
        gFrameBuffer.CreateAttachment("Hair", GL_RGBA8);
        gFrameBuffer.CreateDepthAttachment(GL_DEPTH32F_STENCIL8);

        gWaterReflectionFrameBuffer.Create("Water", 640 * scale, 360 * scale);
        gWaterReflectionFrameBuffer.CreateAttachment("Color", GL_RGBA8);
        gWaterReflectionFrameBuffer.CreateDepthAttachment(GL_DEPTH32F_STENCIL8);

        gWaterRefractionFrameBuffer.Create("Water", 640 * scale, 360 * scale);
        gWaterRefractionFrameBuffer.CreateAttachment("Color", GL_RGBA8);
        gWaterRefractionFrameBuffer.CreateDepthAttachment(GL_DEPTH32F_STENCIL8);

        LoadShaders();
    }

    void RenderFrame() {

        gFrameBuffer.Bind();
        gFrameBuffer.SetViewport();
        unsigned int attachments[4] = {
            gFrameBuffer.GetColorAttachmentSlotByName("Color"),
            gFrameBuffer.GetColorAttachmentSlotByName("Hair"),
            gFrameBuffer.GetColorAttachmentSlotByName("Position"),
            gFrameBuffer.GetColorAttachmentSlotByName("UnderWaterColor") };
        glDrawBuffers(4, attachments);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderLighting();
        RenderHair();
        RenderWater();
        RenderDebug();

        int width, height;
        glfwGetWindowSize(OpenGLBackend::GetWindowPtr(), &width, &height);

        const float waterHeight = Scene::roomY + Scene::waterHeight;
        if (Camera::GetViewPos().y > waterHeight) {
            gFrameBuffer.BlitToDefaultFrameBuffer("Color", 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        } else{
            gFrameBuffer.BlitToDefaultFrameBuffer("UnderWaterColor", 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        glfwSwapBuffers(OpenGLBackend::GetWindowPtr());
        glfwPollEvents();
    }


    void DrawPoint(glm::vec3 position, glm::vec3 color) {
        g_debugPoints.push_back(Vertex(position, color));
    }

    void DrawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color) {
        Vertex v0 = Vertex(begin, color);
        Vertex v1 = Vertex(end, color);
        g_debugLines.push_back(v0);
        g_debugLines.push_back(v1);
    }

    void DrawAABB(AABB& aabb, glm::vec3 color) {
        glm::vec3 FrontTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 FrontBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMax().z);
        glm::vec3 BackTopLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackTopRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMax().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomLeft = glm::vec3(aabb.GetBoundsMin().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        glm::vec3 BackBottomRight = glm::vec3(aabb.GetBoundsMax().x, aabb.GetBoundsMin().y, aabb.GetBoundsMin().z);
        DrawLine(FrontTopLeft, FrontTopRight, color);
        DrawLine(FrontBottomLeft, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackTopRight, color);
        DrawLine(BackBottomLeft, BackBottomRight, color);
        DrawLine(FrontTopLeft, FrontBottomLeft, color);
        DrawLine(FrontTopRight, FrontBottomRight, color);
        DrawLine(BackTopLeft, BackBottomLeft, color);
        DrawLine(BackTopRight, BackBottomRight, color);
        DrawLine(FrontTopLeft, BackTopLeft, color);
        DrawLine(FrontTopRight, BackTopRight, color);
        DrawLine(FrontBottomLeft, BackBottomLeft, color);
        DrawLine(FrontBottomRight, BackBottomRight, color);
    }

    void UpdateDebugLinesMesh() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.insert(std::end(vertices), std::begin(g_debugLines), std::end(g_debugLines));
        g_debugLines.clear();
        for (int i = 0; i < vertices.size(); i++) {
            indices.push_back(i);
        }
        g_debugLinesMesh.UpdateVertexBuffer(vertices, indices);
    }

    void UpdateDebugPointsMesh() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.insert(std::end(vertices), std::begin(g_debugPoints), std::end(g_debugPoints));
        g_debugPoints.clear();
        for (int i = 0; i < vertices.size(); i++) {
            indices.push_back(i);
        }
        g_debugPointsMesh.UpdateVertexBuffer(vertices, indices);
    }


    
    void DrawScene(Shader& shader, bool renderHair) {
        for (GameObject& gameObject : Scene::gGameObjects) {
            if (gameObject.m_meshIndex != MeshManager::GetMeshIndexByName("Water")) {
                OpenGLDetachedMesh* mesh = &MeshManager::gMeshes[gameObject.m_meshIndex];
                if (mesh) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByIndex(gameObject.m_textureIndex)->GetGLTexture().GetID());
                    glBindVertexArray(mesh->GetVAO());
                    glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
                }
            }
        }

        Model* model = AssetManager::GetModelByIndex(0);
        Transform transform;

        transform.position = glm::vec3(3.5, -1.0f, 7.5f);
        transform.rotation.y = 3.14f * 1.7f;
        shader.SetMat4("model", transform.to_mat4());
        for (OpenGLDetachedMesh& mesh : model->m_meshes) {


            shader.SetInt("settings", 0);

            glActiveTexture(GL_TEXTURE0);
            if (mesh.GetName() == "TailFin" ||
                mesh.GetName() == "Tail.001" ||
                mesh.GetName() == "TailBlend") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("MermaidTail")->GetGLTexture().GetID());
                shader.SetInt("settings", 1);
            }
            else if (mesh.GetName() == "Rock2.001") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("Rock")->GetGLTexture().GetID());
            }
            else if (mesh.GetName() == "MermaidEyeLeft" ||
                     mesh.GetName() == "MermaidEyeRight") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("MermaidEye")->GetGLTexture().GetID());
            }
            else if (mesh.GetName() == "MermaidArms.001") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("MermaidArms")->GetGLTexture().GetID());
                shader.SetInt("settings", 2);
            }
            else if (mesh.GetName() == "MermaidFace.001") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("MermaidFace")->GetGLTexture().GetID());
                shader.SetInt("settings", 2);
            }
            else if (mesh.GetName() == "MermaidBody.001") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("MermaidBody")->GetGLTexture().GetID());
                shader.SetInt("settings", 2);
            }
            else if (mesh.GetName() == "Hair1" ||
                mesh.GetName() == "Hair2") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("Hair")->GetGLTexture().GetID());
                shader.SetInt("settings", 3);
                if (!renderHair) {
                    continue;
                }
            }
            else if (mesh.GetName() == "BoobTube") {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("BoobTube")->GetGLTexture().GetID());
            }
            else {
                glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("Gold")->GetGLTexture().GetID());
            }
            glBindVertexArray(mesh.GetVAO());
            glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
        }
        //}
    }

    void RenderHairMesh(Shader& shader, OpenGLDetachedMesh& mesh) {
        shader.SetBool("flipNormals", true);
        glCullFace(GL_FRONT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBindVertexArray(mesh.GetVAO());
        glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);

        shader.SetBool("flipNormals", false);
        glCullFace(GL_BACK);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBindVertexArray(mesh.GetVAO());
        glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
    }

    void RenderHair() {

        gFrameBuffer.Bind();
        gFrameBuffer.SetViewport();
        glDrawBuffer(gFrameBuffer.GetColorAttachmentSlotByName("Hair"));

        Model* model = AssetManager::GetModelByIndex(0);
        Transform transform;
        transform.position = glm::vec3(3.5, -1.0f, 7.5f);
        transform.rotation.y = 3.14f * 1.7f;

        gFrameBuffer.Bind();
        gFrameBuffer.SetViewport();

        gHairMaskShader.Use();
        gHairMaskShader.SetMat4("projection", Camera::GetProjectionMatrix());
        gHairMaskShader.SetMat4("view", Camera::GetViewMatrix());
        gHairMaskShader.SetMat4("model", transform.to_mat4());
        gHairMaskShader.SetVec3("viewPos", Camera::GetViewPos());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("Hair")->GetGLTexture().GetID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("HairOpacity")->GetGLTexture().GetID());

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);         
        for (OpenGLDetachedMesh& mesh : model->m_meshes) {
            if (mesh.GetName() == "Hair2") {
                RenderHairMesh(gHairMaskShader, mesh);
            }
        }        
        for (OpenGLDetachedMesh& mesh : model->m_meshes) {
            if (mesh.GetName() == "Hair1") {
                RenderHairMesh(gHairMaskShader, mesh);
            }
        }    
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("Hair")->GetGLTexture().GetID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gFrameBuffer.GetColorAttachmentHandleByName("Hair"));
        
        glDrawBuffer(gFrameBuffer.GetColorAttachmentSlotByName("Color"));

        gTexturedShader.Use();
        gTexturedShader.SetInt("settings", 4);
        gTexturedShader.SetFloat("viewportWidth", gFrameBuffer.GetWidth());
        gTexturedShader.SetFloat("viewportHeight", gFrameBuffer.GetHeight());

        for (OpenGLDetachedMesh& mesh : model->m_meshes) {
            if (mesh.GetName() == "Hair2") {
                glBindVertexArray(mesh.GetVAO());
                glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }
        for (OpenGLDetachedMesh& mesh : model->m_meshes) {
            if (mesh.GetName() == "Hair1") {
                glBindVertexArray(mesh.GetVAO());
                glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
            }
        }
        gTexturedShader.SetInt("settings", 0);

    }

    void RenderLighting() {

        const float waterHeight = Scene::roomY + Scene::waterHeight;

        gFrameBuffer.Bind();
        gFrameBuffer.SetViewport();
        gFrameBuffer.DrawBuffers({ "Color", "Position" });

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        gTexturedShader.Use();
        gTexturedShader.SetMat4("projection", Camera::GetProjectionMatrix());
        gTexturedShader.SetMat4("view", Camera::GetViewMatrix());
        gTexturedShader.SetMat4("model", glm::mat4(1));
        gTexturedShader.SetVec3("viewPos", Camera::GetViewPos());
        DrawScene(gTexturedShader, false);


        gWaterReflectionFrameBuffer.Bind();
        gWaterReflectionFrameBuffer.SetViewport();
        gWaterReflectionFrameBuffer.DrawBuffers({ "Color" });

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float reflectionDistance = 2 * (Camera::GetViewPos().y - waterHeight);
        Transform reflectionCameraTransform = Camera::GetTransform();
        reflectionCameraTransform.position.y -= reflectionDistance;
        reflectionCameraTransform.rotation.x *= -1;
        glm::mat4 reflectionViewMatrix = glm::inverse(reflectionCameraTransform.to_mat4());

        // glm::mat4 viewMatrix = Camera::GetViewMatrix();
        // glm::mat4 reflectionMatrix = glm::mat4(1.0f);
        // reflectionMatrix[1][1] = -1.0f; // Flip Y-axis
        // reflectionMatrix[3][1] = -2.0f * waterHeight; // Translate below the plane
        // glm::mat4 reflectedView = reflectionMatrix * viewMatrix;
        //

        glEnable(GL_CLIP_DISTANCE0);

        gTexturedShader.Use();
        gTexturedShader.SetMat4("projection", Camera::GetProjectionMatrix());
        gTexturedShader.SetMat4("view", reflectionViewMatrix);
        //gTexturedShader.SetMat4("view", reflectedView);
        gTexturedShader.SetMat4("model", glm::mat4(1));
        gTexturedShader.SetVec3("viewPos", Camera::GetViewPos());
        gTexturedShader.SetVec4("clippingPlane", glm::vec4(0, 1, 0, -waterHeight));

        DrawScene(gTexturedShader, true);

        gWaterRefractionFrameBuffer.Bind();
        gWaterRefractionFrameBuffer.SetViewport();
        gWaterRefractionFrameBuffer.DrawBuffers({ "Color" });
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gTexturedShader.Use();
        gTexturedShader.SetMat4("projection", Camera::GetProjectionMatrix());
        gTexturedShader.SetMat4("view", Camera::GetViewMatrix());
        gTexturedShader.SetMat4("model", glm::mat4(1));
        gTexturedShader.SetVec3("viewPos", Camera::GetViewPos());
        gTexturedShader.SetVec4("clippingPlane", glm::vec4(0, -1, 0, waterHeight));
        DrawScene(gTexturedShader, true);

        glDisable(GL_CLIP_DISTANCE0);


    }

    void RenderWater() {


        static float time = 0;
        static float deltaTime = 0;
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        time += deltaTime;

        gFrameBuffer.Bind();
        gFrameBuffer.SetViewport();
        glDrawBuffer(gFrameBuffer.GetColorAttachmentSlotByName("Color"));

        gWaterShader.Use();
        gWaterShader.SetMat4("projection", Camera::GetProjectionMatrix());
        gWaterShader.SetMat4("view", Camera::GetViewMatrix());
        gWaterShader.SetMat4("model", glm::mat4(1));
        gWaterShader.SetVec3("viewPos", Camera::GetViewPos());
        gWaterShader.SetFloat("viewportWidth", gWaterReflectionFrameBuffer.GetWidth());
        gWaterShader.SetFloat("viewportHeight", gWaterReflectionFrameBuffer.GetHeight());
        gWaterShader.SetFloat("time", time);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gWaterReflectionFrameBuffer.GetColorAttachmentHandleByName("Color"));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gWaterRefractionFrameBuffer.GetColorAttachmentHandleByName("Color"));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("WaterDUDV")->GetGLTexture().GetID());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTextureByName("WaterNormals")->GetGLTexture().GetID());

        OpenGLDetachedMesh* mesh = MeshManager::GetDetachedMeshByName("Water");
        if (mesh) {
            glBindVertexArray(mesh->GetVAO());
            glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }


        const float waterHeight = Scene::roomY + Scene::waterHeight;

        gComputeTest.Use();
        gComputeTest.SetFloat("time", time);
        gComputeTest.SetFloat("viewportWidth", gFrameBuffer.GetWidth());
        gComputeTest.SetFloat("viewportHeight", gFrameBuffer.GetHeight());
        gComputeTest.SetFloat("waterHeight", waterHeight);
        gComputeTest.SetVec3("viewPos", Camera::GetViewPos());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gFrameBuffer.GetColorAttachmentHandleByName("Color"));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gFrameBuffer.GetColorAttachmentHandleByName("Position"));

        glBindImageTexture(2, gFrameBuffer.GetColorAttachmentHandleByName("UnderWaterColor"), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

        glDispatchCompute(gFrameBuffer.GetWidth() / 8, gFrameBuffer.GetHeight() / 8, 1);
    }

    void RenderDebug() {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPointSize(4);
        glDisable(GL_DEPTH_TEST);
        g_solidColorShader.Use();
        g_solidColorShader.SetMat4("projection", Camera::GetProjectionMatrix());
        g_solidColorShader.SetMat4("view", Camera::GetViewMatrix());
        g_solidColorShader.SetMat4("model", glm::mat4(1));
        // Draw lines
        UpdateDebugLinesMesh();
        if (g_debugLinesMesh.GetIndexCount() > 0) {
            glBindVertexArray(g_debugLinesMesh.GetVAO());
            glDrawElements(GL_LINES, g_debugLinesMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
        }
        // Draw points
        UpdateDebugPointsMesh();
        if (g_debugPointsMesh.GetIndexCount() > 0) {
            glBindVertexArray(g_debugPointsMesh.GetVAO());
            glDrawElements(GL_POINTS, g_debugPointsMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
        }
    }

    void LoadShaders() {
        gComputeTest.Load("res/shaders/compute_test.comp");
        gWaterShader.Load("water.vert", "water.frag");
        g_solidColorShader.Load("solid_color.vert", "solid_color.frag");
        gTexturedShader.Load("textured.vert", "textured.frag");
        gHairMaskShader.Load("hair_mask.vert", "hair_mask.frag");
    }

}