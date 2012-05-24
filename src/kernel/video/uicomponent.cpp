/*
** File:	uicomponent.cpp
**
** Author:	Sean Congden
**
** Description:	
*/

extern "C" {
	#include "kmalloc.h"
	#include "defs.h"
	#include "linkedlist.h"
	#include "input/mouse.h"
}

#include "rect.h"

#include "uicomponent.h"


UIComponent::UIComponent(Painter *painter, Rect bounds)
	: painter(painter)
	, bounds(bounds)
	, dirty(true)
{
	children = (LinkedList *)__kmalloc(sizeof(LinkedList));
	list_init(children, __kfree);
}


UIComponent::~UIComponent(void)
{
	list_destroy(children);
	__kfree(children);
}


void UIComponent::Move(Int32 x, Int32 y)
{
	bounds.SetPosition(x, y);

	ListElement *cur_node = list_head(children);
	while (cur_node != NULL) {
		((UIComponent *)list_data(cur_node))->Move(x, y);

		cur_node = cur_node->next;
	}
}


void UIComponent::AddComponent(UIComponent *component)
{
	list_insert_next(children, NULL, component);
}


void UIComponent::Invalidate(void)
{
	dirty = true;

	ListElement *cur_node = list_head(children);
	while (cur_node != NULL) {
		((UIComponent *)list_data(cur_node))->Invalidate();

		cur_node = cur_node->next;
	}
}


void UIComponent::Repaint(void)
{
	Draw();

	ListElement *cur_node = list_head(children);
	while (cur_node != NULL) {
		((UIComponent *)list_data(cur_node))->Repaint();

		cur_node = cur_node->next;
	}

	dirty = false;
}


void UIComponent::HandleMouseEvent(MouseEvent *event)
{
}
