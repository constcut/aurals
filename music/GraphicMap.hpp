#ifndef GRAPHICMAP_H
#define GRAPHICMAP_H

#include <QColor>
#include <QList>
#include <QQuickPaintedItem>

namespace mtherapp {

    struct GraphicElement
    {
      int x,y,w,h;
      QColor color;
      QColor hl;
      int association;
      bool selected;
    };

    class GraphicMap : public QQuickPaintedItem
    {
        Q_OBJECT
    public:
        GraphicMap();

        Q_INVOKABLE int getAssociation(int x, int y);
        //maybe will need getAssociationFront

        Q_INVOKABLE void addElement(int x, int y, int w, int h, QColor c, QColor hl, int association);

        void paint(QPainter *painter);

     //signals:
       Q_SIGNALS:
        void initAssociation(int ass);

    protected:
        QList<GraphicElement> elements;
    };


    class PianoQML : public GraphicMap
    {
        Q_OBJECT
      public:

        PianoQML();

        void addOctave(int x, int y, int startingNote);

        void addNote(int x, int y, int note, bool hasBlack = false);

        Q_INVOKABLE void selectByAssociation(int ass);
        Q_INVOKABLE void unSelectByAssociation(int ass);

    protected:

        GraphicElement *findElementByAssociation(int ass);

        int whiteKeyWidth, whiteKeyHeight, blackKeyWidth, blackKeyHeight;
    };

    //There could be done midi recorder on this principle,
    //also that midi recorder can record data from midi keyboard
}

#endif // GRAPHICMAP_H
