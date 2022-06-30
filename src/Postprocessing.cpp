#include "Postprocessing.h"

void Postprocessing::postprocess(Shader *screenShader, RenderDirection rd)
{
    glDisable(GL_DEPTH_TEST);
    switch (rd)
    {
        case RenderDirection::A_TO_B:
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferB);
            break;
        }
        case RenderDirection::B_TO_A:
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferA);
            break;
        }
        case RenderDirection::A_TO_SCR:
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            break;
        }
        case RenderDirection::B_TO_SCR:
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            break;
        }
    }
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    screenShader->use();
    screenShader->setInt("screenTexture", 0);
    glBindVertexArray(this->quadVAO);
    switch (rd)
    {
        case RenderDirection::A_TO_B:
        {
            glBindTexture(GL_TEXTURE_2D, textureColorbufferA);
            break;
        }
        case RenderDirection::B_TO_A:
        {
            glBindTexture(GL_TEXTURE_2D, textureColorbufferB);
            break;
        }
        case RenderDirection::A_TO_SCR:
        {
            glBindTexture(GL_TEXTURE_2D, textureColorbufferA);
            break;
        }
        case RenderDirection::B_TO_SCR:
        {
            glBindTexture(GL_TEXTURE_2D, textureColorbufferB);
            break;
        }
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Postprocessing::renderQuad()
{
    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Postprocessing::Postprocessing(unsigned int WINDOW_WIDTH, unsigned int WINDOW_HEIGHT)
{
    // Set up Post Processing Quad A
    glGenFramebuffers(1, &this->framebufferA);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebufferA);

    glGenTextures(1, &this->textureColorbufferA);
    glBindTexture(GL_TEXTURE_2D, this->textureColorbufferA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureColorbufferA, 0);

    glGenRenderbuffers(1, &this->rboA);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rboA);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rboA);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    // Set up Post Processing Quad B
    glGenFramebuffers(1, &this->framebufferB);
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebufferB);

    glGenTextures(1, &this->textureColorbufferB);
    glBindTexture(GL_TEXTURE_2D, this->textureColorbufferB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureColorbufferB, 0);

    glGenRenderbuffers(1, &this->rboB);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rboB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rboB);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -------------------------

    glGenVertexArrays(1, &this->quadVAO);
    glGenBuffers(1, &this->quadVBO);
    glBindVertexArray(this->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->quadVertices), &this->quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void Postprocessing::destroy()
{
    glDeleteFramebuffers(1, &framebufferA);
    glDeleteFramebuffers(1, &framebufferB);
    glDeleteRenderbuffers(1, &rboA);
    glDeleteRenderbuffers(1, &rboB);
}
