#ifndef KCANVASROOTPIXMAP_H
#define KCANVASROOTPIXMAP_H

#include <qcanvas.h>

#include <krootpixmap.h>


/**
 * Implement @ref KRootPixmap for @ref QCanvasView.
 *
 * Important note : the pixmap will be set as the background of the
 * @ref QCanvas associated with the view ; this has the following
 * implications :
 * <ul>
 * <li>the given @ref QCanvasView should be positionned at the origin
 *  of the canvas.</li>
 * <li>no other view of the same canvas should use KCanvasRootPixmap.</li>
 * <li>other views of the canvas will have the same background.</li>
 */
class KCanvasRootPixmap : public KRootPixmap
{
 Q_OBJECT
 public:
    KCanvasRootPixmap(QCanvasView *view, const char *name = 0);
    ~KCanvasRootPixmap();

 private slots:
    void backgroundUpdatedSlot(const QPixmap &);

 private:
    QCanvasView *_view;

    class KCanvasRootPixmapPrivate;
    KCanvasRootPixmapPrivate *d;
};

#endif

