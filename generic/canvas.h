#ifndef KCANVASROOTPIXMAP_H
#define KCANVASROOTPIXMAP_H

#include <krootpixmap.h>

class QCanvasView;

/**
 * Implement @ref KRootPixmap for a @ref QCanvasView.
 *
 * The pixmap will be set as the background of the
 * @ref QCanvas associated with the view :
 * <ul>
 * <li>for a correct position of the background pixmap, the given
 * @ref QCanvasView should be positionned at the origin of the canvas.</li>
 * <li>no other view of the same canvas should use @ref KCanvasRootPixmap.</li>
 * <li>other views of the canvas will have the same background pixmap.</li>
 */
class KCanvasRootPixmap : public KRootPixmap
{
 Q_OBJECT
 public:
    KCanvasRootPixmap(QCanvasView *view, const char *name = 0);

 private slots:
    void backgroundUpdatedSlot(const QPixmap &);

 private:
    QCanvasView *_view;

    class KCanvasRootPixmapPrivate;
    KCanvasRootPixmapPrivate *d;
};

#endif

