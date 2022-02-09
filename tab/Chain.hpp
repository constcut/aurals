#ifndef CHAIN_H
#define CHAIN_H 

#include <vector>
#include <memory>


namespace aurals {


    template <typename Child, typename Parent> class ChainContainer
    {
    protected:
        std::vector<std::unique_ptr<Child>> sequence;
        ChainContainer<Child, Parent> *nextOne;
        ChainContainer<Child, Parent> *prevOne;
        Parent *parent;

    public:

        void setParent(Parent *newPa) { parent = newPa;}
        Parent *getParent() { return parent; }

        void setPrev(ChainContainer<Child, Parent> *prev)
        { prevOne=prev;}

        void setNext(ChainContainer<Child, Parent> *next)
        { nextOne=next;}

        ChainContainer<Child, Parent> * getPrev()
        { return prevOne;}

        ChainContainer<Child, Parent> * getNext()
        { return nextOne;}

        virtual ~ChainContainer() {}

        ChainContainer<Child, Parent>& operator=(ChainContainer<Child, Parent> &copy) {
            sequence.clear();
            sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
            return *this;
        }

        ChainContainer<Child, Parent>& operator+=(ChainContainer<Child, Parent> &copy) {
            sequence.insert(sequence.begin(),copy.sequence.begin(),copy.sequence.end());
            return *this;
        }

        ChainContainer():nextOne(0),prevOne(0),parent(0){}

        ChainContainer(int predefinedSize):nextOne(0),prevOne(0),parent(0) {
            sequence.reserve(predefinedSize);
        }

        std::unique_ptr<Child>& operator[](size_t ind) {
            return sequence[ind];
        }

        virtual void push_back(std::unique_ptr<Child> val) {
            sequence.push_back(std::move(val));
        }

        Child* back() {
            return sequence.back().get();
        }

        void pop_back() {
            sequence.pop_back();
        }

        void change(int ind, std::unique_ptr<Child> val) {
            sequence[ind] = std::move(val);
        }

        std::unique_ptr<Child>& at(int ind) {
           return sequence.at(ind);
        }

        const std::unique_ptr<Child>& at(int ind) const {
           return sequence.at(ind);
        }

        size_t size() const {
            return sequence.size();
        }

        void clear() {
            sequence.clear();
        }

        virtual void insertBefore(std::unique_ptr<Child> val, int index=0) {
            sequence.insert(sequence.begin()+index, std::move(val));
        }

        void remove(int index) {
            sequence.erase(sequence.begin()+index);
        }

    };

}

#endif // CHAIN_H
