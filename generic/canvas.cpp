#include "canvas.h"
#include "canvas.moc"

#include <qcanvas.h>


KCanvasRootPixmap::KCanvasRootPixmap(QCanvasView *view, const char *name)
    : KRootPixmap(view, name), _view(view)
{
    setCustomPainting(true);
    connect(this, SIGNAL(backgroundUpdated(const QPixmap &)),
            SLOT(backgroundUpdatedSlot(const QPixmap &)));
}

void KCanvasRootPixmap::backgroundUpdatedSlot(const QPixmap &pixmap)
{
    if ( _view && _view->canvas() )
        _view->canvas()->setBackgroundPixmap(pixmap);
}
