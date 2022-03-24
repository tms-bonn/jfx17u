package com.sun.glass.ui.gtk;

import com.sun.glass.events.KeyEvent;
import com.sun.glass.events.MouseEvent;
import com.sun.glass.events.TouchEvent;
import com.sun.glass.ui.GestureSupport;
import com.sun.glass.ui.TouchInputSupport;
import com.sun.glass.ui.View;
import com.sun.glass.ui.Window;

final class GtkGestureSupport {

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
    private static long singleTouchId;

    public static void notifyBeginTouchEvent(View view, int modifiers,
                                             boolean isDirect,
                                             int touchEventCount) {
        touchSupport.notifyBeginTouchEvent(view, modifiers, isDirect, touchEventCount);
    }

    public static void notifyNextTouchEvent(View view, int state, long id, int x,
                                            int y, int xAbs, int yAbs) {
        if (view instanceof GtkView) {
            GtkView gtkView = (GtkView) view;
            switch (state) {
                case TouchEvent.TOUCH_MOVED:
                    if(singleTouchId == 0 && touchSupport.getTouchCount() == 1) {
                        singleTouchId = id;
                        gtkView.notifyMouse(MouseEvent.DOWN, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    } else if(singleTouchId == id && touchSupport.getTouchCount() == 1) {
                        gtkView.notifyMouse(MouseEvent.DRAG, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    }
                    break;
                case TouchEvent.TOUCH_RELEASED:
                    if(singleTouchId == 0) {
                        singleTouchId = id;
                        gtkView.notifyMouse(MouseEvent.DOWN, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, KeyEvent.MODIFIER_BUTTON_PRIMARY, false, true);
                    }
                    if(singleTouchId == id) {
                        gtkView.notifyMouse(MouseEvent.UP, MouseEvent.BUTTON_LEFT, x, y, xAbs, yAbs, 0, false, true);
                        singleTouchId = 0;
                    }
                    break;
                default:
                    break;
            }
        }

        touchSupport.notifyNextTouchEvent(view, state, id, x, y, xAbs, yAbs);
    }

    public static void notifyEndTouchEvent(View view) {
        touchSupport.notifyEndTouchEvent(view);
        gestureFinished(view, touchSupport.getTouchCount(), false);
    }

    public static void gestureZoomPerformed(View view, int modifiers,
                                            boolean isDirect,
                                            int x, int y, int xAbs,
                                            int yAbs, float scale) {
        GtkGestureSupport.modifiers = modifiers;
        GtkGestureSupport.isDirect = isDirect;

        gestureSupport.handleTotalZooming(view, modifiers, isDirect, false, x,
                y, xAbs, yAbs, scale, 0.0);
    }

    public static void gestureRotatePerformed(View view, int modifiers,
                                            boolean isDirect,
                                            int x, int y, int xAbs,
                                            int yAbs, float rotation) {
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
        GtkGestureSupport.modifiers = modifiers;
        GtkGestureSupport.isDirect = isDirect;

        if(touchSupport.getTouchCount() == 1) {
            Window window = view.getWindow();
            // Prevent scrolling when the touch moves out of the window
            if(xAbs <= window.getX() + window.getWidth() && yAbs <= window.getY() + window.getHeight()) {
                gestureSupport.handleTotalScrolling(view, modifiers, isDirect, false, 1, x,
                        y, xAbs, yAbs, offsetX, offsetY, multiplier, multiplier);
            }
        }
    }

    private static void gestureFinished(View view, int touchCount,
                                        boolean isInertia) {
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
