#include "textclass.h"

TextClass::TextClass()
{
    m_Font = 0;
    m_FontShader = 0;
    m_sentence1 = 0;
    m_sentence2 = 0;
}


TextClass::TextClass(const TextClass& /*other*/)
{

}


TextClass::~TextClass()
{

}


bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight,
    DirectX::XMMATRIX baseViewMatrix)
{
    bool result = true;

    //Store the screen width and height
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    //Store the base view matrix
    m_baseViewMatrix = baseViewMatrix;

    //Create the font object
    m_Font = new FontClass();

    //Initialize the font object
    result = m_Font->Initialize(device, deviceContext, "assets/fonts/fontdata.txt", "assets/fonts/font.tga");
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
        return false;
    }

    //Create the font shader object
    m_FontShader = new FontShaderClass();

    //Initialize the font shader object
    result = m_FontShader->Initialize(device, hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
        return false;
    }

    //Initialize the first sentence
    result = InitializeSentence(&m_sentence1, 16, device);
    if (!result)
    {
        return false;
    }

    //Now update the sentence vertex buffer with the new string information
    result = UpdateSentence(m_sentence1, "Hello", 100, 100, 1.0f, 1.0f, 1.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    //Initialize the first sentence
    result = InitializeSentence(&m_sentence2, 16, device);
    if (!result)
    {
        return false;
    }

    //Update the sentence vertex buffer with the new string information
    result = UpdateSentence(m_sentence2, "Goodbye", 100, 200, 1.0f, 1.0f, 0.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}


void TextClass::Shutdown()
{
    //Release the first sentence
    ReleaseSentence(&m_sentence1);

    //Release the second sentence
    ReleaseSentence(&m_sentence2);

    //Release the font shader object
    if (m_FontShader)
    {
        m_FontShader->Shutdown();
        delete m_FontShader;
        m_FontShader = 0;
    }

    //Release the font object
    if (m_Font)
    {
        m_Font->Shutdown();
        delete m_Font;
        m_Font = 0;
    }
}


bool TextClass::SetFps(int fps, ID3D11DeviceContext* deviceContext)
{
    char tempString[16] = {};
    char fpsString[16] = {};
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    bool result = true;

    //Truncate the FPS to below 10,000
    if (fps > 9999)
    {
        fps = 9999;
    }

    //Convert the FPS int to string format
    _itoa_s(fps, tempString, 10);

    //Set up the FPS string
    strcpy_s(fpsString, "FPS: ");
    strcat_s(fpsString, tempString);

    //If FPS is 60 or above set the FPS color to green
    if (fps >= 60)
    {
        red = 0.0f;
        green = 1.0f;
        blue = 0.0f;
    }

    //If FPS is below 60 set the fps color to yellow
    if (fps < 60)
    {
        red = 1.0f;
        green = 1.0f;
        blue = 0.0f;
    }

    //If FPS is below 30 set the fps color to red
    if (fps < 30)
    {
        red = 1.0f;
        green = 0.0f;
        blue = 0.0f;
    }

    //Update the sentence vertex buffer with the new string info
    result = UpdateSentence(m_sentence1, fpsString, 20, 20, red, green, blue, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}


bool TextClass::SetCpu(int cpu, ID3D11DeviceContext* deviceContext)
{
    char tempString[16] = {};
    char cpuString[16] = {};
    bool result = true;

    //Convert the CPU int to string format
    _itoa_s(cpu, tempString, 10);

    //Set up the CPU string
    strcpy_s(cpuString, "CPU: ");
    strcat_s(cpuString, tempString);
    strcat_s(cpuString, "%");

    //Update the sentence vertex buffer with new string info
    result = UpdateSentence(m_sentence2, cpuString, 20, 40, 0.0f, 1.0f, 0.0f, deviceContext);
    if (!result)
    {
        return false;
    }

    return true;
}


bool TextClass::Render(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX orthoMatrix)
{
    bool result = true;

    //Draw the first sentence
    result = RenderSentence(deviceContext, m_sentence1, worldMatrix, orthoMatrix);
    if (!result)
    {
        return false;
    }

    //Draw the second sentence
    result = RenderSentence(deviceContext, m_sentence2, worldMatrix, orthoMatrix);
    if (!result)
    {
        return false;
    }

    return true;
}


bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
    VertexType* vertices = nullptr;
    unsigned long* indices = nullptr;
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    D3D11_BUFFER_DESC indexBufferDesc = {};
    D3D11_SUBRESOURCE_DATA vertexData = {};
    D3D11_SUBRESOURCE_DATA indexData = {};
    HRESULT result = 0;

    //Create a new sentence object
    *sentence = new SentenceType();

    //Initialize the sentence buffers to null
    (*sentence)->vertexBuffer = 0;
    (*sentence)->indexBuffer = 0;

    //Set the maximum length of the sentence
    (*sentence)->maxLength = maxLength;

    //Set the number of vertices in the vertex array
    (*sentence)->vertexCount = 6 * maxLength;

    //Set the number of indexes in the index array
    (*sentence)->indexCount = (*sentence)->vertexCount;

    //Create the vertex array
    vertices = new VertexType[(*sentence)->vertexCount];

    //Create the index array
    indices = new unsigned long[(*sentence)->indexCount];

    // Initialize the index array.
    for (int i = 0; i < (*sentence)->indexCount; i++)
    {
        indices[i] = i;
    }

    //Set up the description of the dynamic vertex buffer
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    //Give the subresource structure a pointer to the vertex data
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    //Create the vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    //Set up the description of the static index buffer
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    //Give the subresource structure a pointer to the index data
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    //Create the index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
    if (FAILED(result))
    {
        return false;
    }

    //Release the vertex array as it is no longer needed
    delete[] vertices;
    vertices = 0;

    //Release the index array as it is no longer needed
    delete[] indices;
    indices = 0;

    return true;
}


bool TextClass::UpdateSentence(SentenceType* sentence, char* text, int positionX, int positionY, float red, float green, float blue,
    ID3D11DeviceContext* deviceContext)
{
    int numLetters = 0;
    VertexType* vertices = nullptr;
    float drawX = 0.0f;
    float drawY = 0.0f;
    HRESULT result = 0;
    D3D11_MAPPED_SUBRESOURCE mappedResource = {};
    VertexType* verticesPtr = nullptr;

    //Store the color of the sentence
    sentence->red = red;
    sentence->green = green;
    sentence->blue = blue;

    //Get the number of letters in the sentence
    numLetters = (int)strlen(text);

    //Check for possible buffer overflow
    if (numLetters > sentence->maxLength)
    {
        return false;
    }

    //Create the vertex array
    vertices = new VertexType[sentence->vertexCount];

    //Calculate the X and Y pixel position on the screen to start drawing to
    drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
    drawY = (float)((m_screenHeight / 2) - positionY);

    //Use the font class to build the vertex array from the sentence text and sentence draw location
    m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

    //Lock the vertex buffer so it can be written to
    result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    //Get a pointer to the data in the vertex buffer
    verticesPtr = (VertexType*)mappedResource.pData;

    //Copy the data into the vertex buffer
    memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

    //Unlock the vertex buffer
    deviceContext->Unmap(sentence->vertexBuffer, 0);

    //Release the vertex array as it is no longer needed
    delete[] vertices;
    vertices = 0;

    return true;
}


void TextClass::ReleaseSentence(SentenceType** sentence)
{
    if (*sentence)
    {
        //Release the sentence vertex buffer
        if ((*sentence)->vertexBuffer)
        {
            (*sentence)->vertexBuffer->Release();
            (*sentence)->vertexBuffer = 0;
        }

        //Release the sentence index buffer
        if ((*sentence)->indexBuffer)
        {
            (*sentence)->indexBuffer->Release();
            (*sentence)->indexBuffer = 0;
        }

        //Release the sentence
        delete *sentence;
        *sentence = 0;
    }
}


bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, DirectX::XMMATRIX worldMatrix,
    DirectX::XMMATRIX orthoMatrix)
{
    unsigned int stride = sizeof(VertexType);;
    unsigned int offset = 0;
    DirectX::XMFLOAT4 pixelColor = {};
    bool result = true;

    //Set the vertex buffer to active in the input assembler so it can be rendered
    deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    //Set the index buffer to active in the input assembler so it can be rendered
    deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Set the type of primitive that should be rendered from this vertex buffer, in this case triangles
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Create a pixel color vector with the input sentence color
    pixelColor = DirectX::XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

    //Render the text using the font shader
    result = m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(),
        pixelColor);
    if (!result)
    {
        return false;
    }

    return true;
}