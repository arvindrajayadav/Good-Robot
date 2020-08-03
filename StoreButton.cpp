#include "master.h"
#include "StoreButton.h"

using namespace pyrodactyl;

void StoreButton::Load(rapidxml::xml_node<char> *node, pyroRect *parent, const bool &echo)
{
	Button::Load(node, parent, echo);

	if (NodeValid("buy", node))
	{
		rapidxml::xml_node<char> *buynode = node->first_node("buy");
		buy.Load(buynode, this);

		if (NodeValid("price", buynode))
			price.Load(buynode->first_node("price"), &buy);

		//The default description text is drawn when the buy button is unavailable
		buy_disabled_text = price.text;
	}

	if (NodeValid("img", node))
		img.Load(node->first_node("img"), this);
}

void StoreButton::Init(const StoreButton &ref, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	Button::Init(ref, parent, XOffset, YOffset);
	buy.Init(ref.buy, this);
	img.Init(ref.img, this);

	price.Init(ref.price, &buy);
	buy_disabled_text = ref.buy_disabled_text;
}

ButtonAction StoreButton::HandleEvents()
{
	Button::HandleEvents();
	return buy.HandleEvents();
}

void StoreButton::Draw()
{
	Button::Draw();

	if (draw_img)
		RenderSprite(img.x, img.y, img.w, img.h, sprite, color);

	buy.Draw();
	price.Draw();
}

void StoreButton::SetCostText(bool condition, int cost, long player_money, SpriteEntry s, GLrgba col)
{
	if (s == SPRITE_INVALID)
		draw_img = false;
	else
	{
		sprite = s;
		color = col;
		draw_img = true;
	}

	if (condition)
	{
		price.text = buy_disabled_text;
		buy.Visible(false);
	}
	else
	{
		price.text = NumberToString(cost);

		//Only show buy button if the player can afford it
		buy.Visible(player_money >= cost);
	}
}

void StoreButton::SetUI(pyroRect *parent)
{
	Button::SetUI(parent);
	buy.SetUI(this);
	img.SetUI(this);
	price.SetUI(&buy);
}