#include "CharacterRenderer.h"
#include "GameCharacter.h"
#include "CharacterFrame.h"
#include "CharacterSprite.h"
#include "Object.h"
#include "Material.h"
#include "Texture.h"
#include "Transform.h"
#include "Rendering.h"

void CharacterRenderer::OnRenderObjects()
{
	if(_owner == nullptr || _owner->transform() == nullptr)
	{
		this->enabled = false;
		return;
	}

	if(_character == nullptr)
	{
		_character = _owner->GetBehaviour<GameCharacter>("GameCharacter");
		if(_character == nullptr)
		{
			this->enabled = false;
			return;
		}
	}
	
	const CharacterSprite* frameData = _character->currentFrame()->frameData();
	Texture* frameTex = Resource::GetTexture(frameData->sprite());

	HandleRenderingMaterial(frameData, frameTex);
	HandleRenderingTransform(frameData, frameTex);

	Rendering::DrawMesh(_renderingTransform.get(), _mesh, _material);
}

void CharacterRenderer::HandleRenderingMaterial(const CharacterSprite* spriteData, Texture* texture)
{
	if(texture == nullptr)
	{
		return;
	}

	glm::ivec4 pixelRect = spriteData->pixelRect();

	glm::vec4 params(
		pixelRect.x * texture->size.z,
		pixelRect.y * texture->size.w,
		(pixelRect.z - pixelRect.x) * texture->size.z,
		(pixelRect.w - pixelRect.y) * texture->size.w
	);

	_material->uniformTextures.insert_or_assign("tex0", MaterialTexture(texture, params));
}

void CharacterRenderer::HandleRenderingTransform(const CharacterSprite* spriteData, Texture* texture)
{
	glm::ivec2 originOffset = spriteData->originOffset();
	glm::vec2 sizeMultiplier = _character->sizeMultiplier();

	_renderingTransform->SnapTo(*_owner->transform());

	_renderingTransform->position += glm::ivec2(
		glm::round(originOffset.x * sizeMultiplier.x),
		glm::round(originOffset.y * sizeMultiplier.y)
	);

	_renderingTransform->scale = glm::vec2(
		texture->size.x * sizeMultiplier.x,
		texture->size.y* sizeMultiplier.y);
}

CharacterRenderer::CharacterRenderer(Object* owner, const json& j) : Renderer(owner, j)
{
	if(_material == nullptr)
	{
		this->enabled = false;
		return;
	}

	_material = Resource::CopyMaterial(_material);
	_renderingTransform = std::make_unique<Transform>(_owner);
}

CharacterRenderer::~CharacterRenderer()
{
}
