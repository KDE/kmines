#include "canvas.h"
#include "canvas.moc"


KCanvasRootPixmap::KCanvasRootPixmap(QCanvasView *view, const char *name)
    : KRootPixmap(view, name), _view(view)
{
    setCustomPainting(true);
    connect(this, SIGNAL(backgroundUpdated(const QPixmap &)),
            SLOT(backgroundUpdatedSlot(const QPixmap &)));
}

KCanvasRootPixmap::~KCanvasRootPixmap()
{}

void KCanvasRootPixmap::backgroundUpdatedSlot(const QPixmap &pixmap)
{
    if ( _view->canvas()==0 ) return;
    _view->canvas()->setBackgroundPixmap(pixmap);
}
