package com.sun.glass.ui.gtk;

import com.sun.glass.events.KeyEvent;
import com.sun.glass.events.MouseEvent;
import com.sun.glass.events.TouchEvent;
import com.sun.glass.ui.GestureSupport;
import com.sun.glass.ui.TouchInputSupport;
import com.sun.glass.ui.View;
import com.sun.glass.ui.Window;

import java.util.logging.Logger;

final class GtkGestureSupport {

    private static final Logger LOGGER = Logger.getLogger(GtkGestureSupport.class.getName());

    private native static void _initIDs();

    static {
        _initIDs();
    }
    // The multiplier used to convert scroll units to pixels
    private static final double multiplier = 1.0;

    private final static GestureSupport gestureSupport = new GestureSupport(false);
    private final static TouchInputSupport touchSupport = new TouchInputSupport(gestureSupport.createTouchCountListener(), true);
    private static int modifiers;
    private static boolean isDirect;
    private static int touchPressedXAbs;
    private static int touchPressedYAbs;

    public static void notifyBeginTouchEvent(View view, int modifiers, boolean isDirect, long id,
                                             int touchEventCount) {
        LOGGER.fine("notifyBeginTouchEvent: view: " + view + ", modifiers: " + modifiers + ", isDirect: " + isDirect + ", id: " + id + ", touchEventCount: " + touchEventCount);
        GtkGestureSupport.modifiers = modifiers;
        touchPressedXAbs = 0;
        touchPressedYAbs = 0;
        touchSupport.notifyBeginTouchEvent(view, modifiers, isDirect, touchEventCount);
    }

    public static void notifyNextTouchEvent(View view, int state, long id, int x,
                                            int y, int xAbs, int yAbs) {

        LOGGER.fine("notifyNextTouchEvent: view: " + view + " state: " + state + " id: " + id + " x: " + x + " y: " + y + " xAbs: " + xAbs + " yAbs: " + yAbs);

        /*
            Some touch monitors deliver touch_moved even when the finger is not moving.
            This means that row selection clicks are not recognized.
            This is recognized here and unnecessary drag events are avoided.
         */
        if(state == TouchEvent.TOUCH_MOVED && xAbs == touchPressedXAbs && yAbs == touchPressedYAbs && touchSupport.getTouchCount() < 2)
        {
            LOGGER.fine("notifyNextTouchEvent will be ignored");
            return;
        }

        touchSupport.notifyNextTouchEvent(view, state, id, x, y, xAbs, yAbs);

        if (view instanceof GtkView && !gestureSupport.isRotating() && !gestureSupport.isZooming() && touchSupport.getTouchCount() < 2) {
            GtkView gtkView = (GtkView) view;
            switch (state) {
                case TouchEvent.TOUCH_PRESSED:
                    touchPressedXAbs = xAbs;
                    touchPressedYAbs = yAbs;
                    gtkView.notifyMouse(MouseEvent.DOWN, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, modifiers | KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    break;
                case TouchEvent.TOUCH_MOVED:
                    gtkView.notifyMouse(MouseEvent.DRAG, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, modifiers | KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    break;
                case TouchEvent.TOUCH_RELEASED:
                    touchPressedXAbs = 0;
                    touchPressedYAbs = 0;
                    gtkView.notifyMouse(MouseEvent.UP, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, modifiers == 0 ? modifiers : modifiers ^ KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    break;
                default:
                    break;
            }
        }
    }

    public static void notifyEndTouchEvent(View view, long id) {
        LOGGER.fine("notifyEndTouchEvent: view: " + view + ", id: " + id);
        touchPressedXAbs = 0;
        touchPressedYAbs = 0;
        touchSupport.notifyEndTouchEvent(view);
        gestureFinished(view, touchSupport.getTouchCount(), false);
    }

    public static void gestureReleaseTouchEvents(View view)
    {
        LOGGER.fine("gestureReleaseTouchEvents: view: " + view);
        touchSupport.releaseTouchEvents(view);
    }

    public static void gestureZoomPerformed(View view, int modifiers,
                                            boolean isDirect,
                                            int x, int y, int xAbs,
                                            int yAbs, float scale) {
        LOGGER.fine("gestureZoomPerformed: view: " + view + " modifiers: " + modifiers + " isDirect: " + isDirect + " x: " + x + " y: " + y + " xAbs: " + xAbs + " yAbs: " + yAbs + " scale: " + scale);
        GtkGestureSupport.modifiers = modifiers;
        GtkGestureSupport.isDirect = isDirect;

        gestureSupport.handleTotalZooming(view, modifiers, isDirect, false, x,
                y, xAbs, yAbs, scale, 0.0);
    }

    public static void gestureRotatePerformed(View view, int modifiers,
                                            boolean isDirect,
                                            int x, int y, int xAbs,
                                            int yAbs, float rotation) {
        LOGGER.fine("gestureRotatePerformed: view: " + view + " modifiers: " + modifiers + " isDirect: " + isDirect + " x: " + x + " y: " + y + " xAbs: " + xAbs + " yAbs: " + yAbs + " rotation: " + rotation);
        GtkGestureSupport.modifiers = modifiers;
        GtkGestureSupport.isDirect = isDirect;

        gestureSupport.handleTotalRotation(view, modifiers, isDirect, false, x,
                y, xAbs, yAbs, Math.toDegrees(
                        rotation));
    }

    public static void gestureDragUpdatePerformed(View view, int modifiers,
                                            boolean isDirect,
                                            int x, int y, int xAbs,
                                            int yAbs, float offsetX, float offsetY) {
        LOGGER.fine("gestureDragUpdatePerformed: view: " + view + " modifiers: " + modifiers + " isDirect: " + isDirect + " x: " + x + " y: " + y + " xAbs: " + xAbs + " yAbs: " + yAbs + " offsetX: " + offsetX + " offsetY: " + offsetY);
        GtkGestureSupport.modifiers = modifiers;
        GtkGestureSupport.isDirect = isDirect;

        if(offsetX == 0 && offsetY == 0)
        {
            // no drag
            return;
        }

        if(touchSupport.getTouchCount() == 1) {
            Window window = view.getWindow();
            // Prevent scrolling when the touch moves out of the window
            if(xAbs <= window.getX() + window.getWidth() && yAbs <= window.getY() + window.getHeight()) {
                gestureSupport.handleTotalScrolling(view, modifiers, isDirect, false, 1, x,
                        y, xAbs, yAbs, offsetX, offsetY, multiplier, multiplier);
            }
        }
    }

    private static void gestureFinished(View view, int touchCount, boolean isInertia) {
        LOGGER.fine("gestureFinished: view: " + view + " touchCount: " + touchCount + " isInertia: " + isInertia);

        if (view == null) {
            return;
        }

        if (gestureSupport.isScrolling() && touchCount == 0) {
            gestureSupport.handleScrollingEnd(view, modifiers, touchCount, isDirect,
                    isInertia,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE);
        }

        if (gestureSupport.isRotating() && touchCount < 2) {
            gestureSupport.handleRotationEnd(view, modifiers, isDirect, isInertia,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE);
        }

        if (gestureSupport.isZooming() && touchCount < 2) {
            gestureSupport.handleZoomingEnd(view, modifiers, isDirect, isInertia,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE,
                    View.GESTURE_NO_VALUE);
        }
    }
}
