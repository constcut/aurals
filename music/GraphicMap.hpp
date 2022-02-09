#ifndef GRAPHICMAP_H
#define GRAPHICMAP_H

#include <QColor>
#include <QList>
#include <QQuickPaintedItem>

namespace aurals {

    struct GraphicElement  {
      int x,y,w,h;
      QColor color;
      QColor hl;
      int association;
      bool selected;
    };

    class GraphicMap : public QQuickPaintedItem {
        Q_OBJECT
    public:
        GraphicMap() = default;

        Q_INVOKABLE int getAssociation(const int x, const int y) const;
        Q_INVOKABLE void addElement(const int x, const int y, const int w, const int h,
                                    const QColor c, const QColor hl, const int association);

        void paint(QPainter *painter) override;

       Q_SIGNALS:
        void initAssociation(int ass);

    protected:
        QList<GraphicElement> elements;
    };


    class PianoQML : public GraphicMap {
        Q_OBJECT
      public:

        PianoQML();

        void addOctave(const int x, const int y, const int startingNote);
        void addNote(const int x, const int y, const int note, const bool hasBlack = false);

        Q_INVOKABLE void selectByAssociation(const int ass);
        Q_INVOKABLE void unSelectByAssociation(const int ass);

    protected:

        GraphicElement* findElementByAssociation(const int ass);

        int _whiteKeyWidth;
        int _whiteKeyHeight;
        int _blackKeyWidth;
        int _blackKeyHeight;
    };

}

#endif // GRAPHICMAP_H
