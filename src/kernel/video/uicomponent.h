/*
** File:	uicomponent.h
**
** Author:	Sean Congden
**
** Description:	
*/

#ifndef _UI_COMPONENT_H
#define _UI_COMPONENT_H

extern "C" {
	#include "linkedlist.h"
	#include "input/mouse.h"
}

#include "painter.h"

class UIComponent {
public:

	/*
	** UIComponent(painter,bounds)
	**
	** Creates a new component to be used in a graphical
	** application.  The component uses the provided painter to
	** draw its graphics and is clipped against the bounding
	** rectangle provided.
	*/
	UIComponent(Painter *painter, Rect bounds);
	virtual ~UIComponent(void);

	/*
	** AddComonent(component)
	**
	** Adds the given component as a child to this component.
	** The new component will be updated and redrawn when its
	** parent is updated and drawn.
	*/
	void AddComponent(UIComponent *component);

	/*
	** Move(x,y)
	**
	** Moves the component on screen placing the upper left
	** corner of the component at the provided screen space
	** coordinates.
	*/
	virtual void Move(Int32 x, Int32 y);

	/*
	** Invalidate(event)
	**
	** Marks the component indicating the component should be
	** repainted.  Should be called after updating the contents
	** of the component.
	*/
	void Invalidate(void);

	/*
	** GetBounds()
	**
	** Gives the bounding rectangle of the component
	**
	** Returns:
	**      the bounding rectangle of the component
	*/
	Rect& GetBounds(void) { return bounds; }
	
	/*
	** IsDirty()
	**
	** Indicates if the component has recently been updated and
	** needs to be repainted.
	**
	** Returns:
	**      if the component should be repainted
	*/
	bool IsDirty(void) { return dirty; }

	/*
	** HandleMouseEvent(event)
	**
	** Handles a mouse event generated when the window has
	** focus at the given x and y coordinates.
	*/
	virtual void HandleMouseEvent(MouseEvent *event);

protected:

	/*
	** Draw()
	**
	** Handles drawing the component.  This method should only
	** be used to draw the current component.  Repainting is
	** initiated with the Repaint() method which automatically
	** calls Draw() subcomponents.
	*/
	virtual void Draw(void) = 0;

	Painter *painter;
	Rect bounds;

private:

	/*
	** Repaint()
	**
	** Initiates drawing of the component and its subcomponents.
	** This method should not be called directly.
	** To redraw a component, use the Invalidate() method.
	*/
	void Repaint(void);

	LinkedList *children;
	bool dirty;

	// Desktop should be the only class allowed to call Repaint().  Other
	// classes should call Invalidate() to mark the compontent as dirty.
	friend class Desktop;
};

#endif
