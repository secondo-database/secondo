/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

//
// Created by gstancul on 14.06.19.
//

#include "KafkaConsumer.h"
#include "KafkaProducer.h"


extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace kafka {


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

    class SCircle {
    public:
// constructor doing nothing
        SCircle() {}

// constructor initializing the object
        SCircle(const double _x, const double _y, const double _r) :
                x(_x), y(_y), r(_r) {}

// copy constructor
        SCircle(const SCircle &src) : x(src.x), y(src.y), r(src.r) {}

// assignment operator
        SCircle &operator=(const SCircle &src) {
            x = src.x;
            y = src.y;
            r = src.r;
            return *this;
        }

// destructor
        ~SCircle() {}


        static const string BasicType() { return "scircleGst"; }

// the checktype function for non-nested types looks always
// the same
        static const bool checkType(const ListExpr list) {
            return listutils::isSymbol(list, BasicType());
        }

        double perimeter() const {
            return 2 * M_PI * r;
        }

        double getX() const { return x; }

        double getY() const { return y; }

        double getR() const { return r; }

    private:
        double x;
        double y;
        double r;


    public:

    };


    ListExpr SCircleProperty() {
        return (nl->TwoElemList(
                nl->FourElemList(
                        nl->StringAtom("Signature"),
                        nl->StringAtom("Example Type List"),
                        nl->StringAtom("List Rep"),
                        nl->StringAtom("Example List")),
                nl->FourElemList(
                        nl->StringAtom("-> SIMPLE"),
                        nl->StringAtom(SCircle::BasicType()),
                        nl->StringAtom("(real real real) = (x,y,r)"),
                        nl->StringAtom("(13.5 -76.0 1.0)")
                )));
    }

    Word InSCircle(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr &errorInfo, bool &correct) {

        cout << "Hello world GST: Here we are.";
// create a result with addr pointing to 0
        Word res((void *) 0);
// assume an incorrect list
        correct = false;
// check whether the list has three elements
        if (!nl->HasLength(instance, 3)) {
            cmsg.inFunError("expected three numbers");
            return res;
        }
// check whether all elements are numeric
        if (!listutils::isNumeric(nl->First(instance))
            || !listutils::isNumeric(nl->Second(instance))
            || !listutils::isNumeric(nl->Third(instance))) {
            cmsg.inFunError("expected three numbers");
            return res;
        }
// get the numeric values of the elements
        double x = listutils::getNumValue(nl->First(instance));
        double y = listutils::getNumValue(nl->Second(instance));
        double r = listutils::getNumValue(nl->Third(instance));
// check for a valid radius
        if (r <= 0) {
            cmsg.inFunError("invalid radius (<=0)");
            return res;
        }
// list was correct, create the result
        correct = true;
        res.addr = new SCircle(x, y, r);
        return res;

    }

    ListExpr OutSCircle(ListExpr typeInfo, Word value) {
        SCircle *k = (SCircle *) value.addr;
        return nl->ThreeElemList(
                nl->RealAtom(k->getX()),
                nl->RealAtom(k->getY()),
                nl->RealAtom(k->getR()));
    }

    Word CreateSCircle(const ListExpr typeInfo) {
        Word w;
        w.addr = (new SCircle(0, 0, 1.0));
        return w;
    }

    void DeleteSCircle(const ListExpr typeInfo, Word &w) {
        SCircle *k = (SCircle *) w.addr;
        delete k;
        w.addr = 0;
    }

    bool OpenSCircle(SmiRecord &valueRecord,
                     size_t &offset, const ListExpr typeInfo,
                     Word &value) {
        size_t size = sizeof(double);
        double x, y, r;
        bool ok = (valueRecord.Read(&x, size, offset) == size);
        offset += size;
        ok = ok && (valueRecord.Read(&y, size, offset) == size);
        offset += size;
        ok = ok && (valueRecord.Read(&r, size, offset) == size);
        offset += size;
        if (ok) {
            value.addr = new SCircle(x, y, r);
        } else {
            value.addr = 0;
        }
        return ok;
    }

    bool SaveSCircle(SmiRecord &valueRecord, size_t &offset,
                     const ListExpr typeInfo, Word &value) {
        SCircle *k = static_cast <SCircle *>( value.addr );
        size_t size = sizeof(double);
        double v = k->getX();
        bool ok = valueRecord.Write(&v, size, offset);
        offset += size;
        v = k->getY();
        ok = ok && valueRecord.Write(&v, size, offset);
        offset += size;
        v = k->getR();
        ok = ok && valueRecord.Write(&v, size, offset);
        offset += size;
        return ok;
    }

    void CloseSCircle(const ListExpr typeInfo, Word &w) {
        SCircle *k = (SCircle *) w.addr;
        delete k;
        w.addr = 0;
    }

    Word CloneSCircle(const ListExpr typeInfo, const Word &w) {
        SCircle *k = (SCircle *) w.addr;
        Word res;
        res.addr = new SCircle(k->getX(), k->getY(), k->getR());
        return res;
    }

    void *CastSCircle(void *addr) {
        return (new(addr) SCircle);
    }

    bool SCircleTypeCheck(ListExpr type, ListExpr &errorInfo) {
        return nl->IsEqual(type, SCircle::BasicType());
    }

    int SizeOfSCircle() {
        return 3 * sizeof(double);
    }

    TypeConstructor SCircleTC(
            SCircle::BasicType(), // name of the type
            SCircleProperty, // property function
            OutSCircle, InSCircle, // out and in function
            0, 0, // deprecated, don’t think about it
            CreateSCircle, DeleteSCircle, // creation and deletion
            OpenSCircle, SaveSCircle, // open and save functions
            CloseSCircle, CloneSCircle, // close and clone functions
            CastSCircle, // cast function
            SizeOfSCircle, // sizeOf function
            SCircleTypeCheck); // type checking function




    ListExpr perimeterTM(ListExpr args) {
        string err = "scircle expected";
// check the number of arguments
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
// check type of the argument
        if (!SCircle::checkType(nl->First(args))) {
            return listutils::typeError(err);
        }
// return the result type
        return listutils::basicSymbol<CcReal>();
    }

// @formatter:off
    int perimeterVM(Word *args, Word &result, int message, Word &local,
                    Supplier s) {
        SCircle *k = (SCircle *) args[0].addr; // get the argument and cast it
        result = qp->ResultStorage(s); // use the result storage
        CcReal *res = (CcReal *) result.addr; // cast the result
        res->Set(true, k->

                perimeter()

        ); // compute and set the result
        return 0;
    }

// @formatter:on

    OperatorSpec perimeterSpec(
            "scircle -> real",
            "perimeter(_)",
            "Computes the perimeter of a disc. GST",
            "query perimeterGst([const scircleGst value (1.0 8.0 16.0)])"
    );

    Operator perimeterOp(
            "perimeterGst", // name of the operator
            perimeterSpec.getStr(), // specification
            perimeterVM, // value mapping
            Operator::SimpleSelect, // selection function
            perimeterTM // type mapping
    );


/*
6 Streams as Both, Arguments and Results

Some operators have a stream as an argument and return also a stream.  The
implementation combines stream consuming with stream producing operators.

We show as an example the operator ~startsWithS~. This is a kind of filter operator.
It receives a stream of strings and a single string argument. All elements in the
stream starting with the second argument pass the operator, all others are
filtered out.

6.1 Type Mapping

The type mapping is quite usual.

*/
    ListExpr startsWithSTM(ListExpr args) {
        if (!nl->HasLength(args, 2)) {
            return listutils::typeError("wrong number of args");
        }
        if (!Stream<CcString>::checkType(nl->First(args))
            || !CcString::checkType(nl->Second(args))) {
            return listutils::typeError("stream(string) x string expected");
        }
        return nl->First(args);
    }

/*
6.2 LocalInfo Class

As for other stream operators, we create a local info class storing the
state of this operator and computing the next result element.

Because we create an instance of this class in case of a OPEN message
and delete the instance in case of a CLOSE message, we open the
argument stream in the constructor and close it in the destructor.

Elements passing the test are just returned as the next result.
Filtered out strings  are deleted.

*/
    class startsWithSLI {
    public:

        // s is the stream argument, st the string argument
        startsWithSLI(Word s, CcString *st) : stream(s), start("") {
            def = st->IsDefined();
            if (def) { start = st->GetValue(); }
            stream.open();
        }

        ~startsWithSLI() {
            stream.close();
        }

        CcString *getNext() {
            if (!def) { return 0; }
            CcString *k;
            while ((k = stream.request())) {
                if (k->IsDefined() &&
                    stringutils::startsWith(k->GetValue(), start)) {
                    return k;
                }
                k->DeleteIfAllowed();
            }
            return 0;
        }

    private:
        Stream<CcString> stream;
        string start;
        bool def;
    };

/*
6.3 Value Mapping

Because the complete functionality is outsourced to the ~LocalInfo~ class,
the implementation of the actual value mapping is straightforward.

*/
    int startsWithSVM(Word *args, Word &result, int message,
                      Word &local, Supplier s) {
        startsWithSLI *li = (startsWithSLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new startsWithSLI(args[0],
                                               (CcString *) args[1].addr);
                return 0;
            case REQUEST:
                result.addr = li ? li->getNext() : 0;
                return result.addr ? YIELD : CANCEL;
            case CLOSE:
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                return 0;
        }
        return 0;
    }

/*
6.4 Specification

*/

    OperatorSpec startsWithSSpec(
            " stream(string) x string -> stream(string)",
            " _ startsWithGst[_]",
            " All strings in the stream not starting with the second "
            " are filtered out from the stream",
            " query plz feed projecttransformstream[Ort] "
            "startsWithGst(\"Ha\") count"
    );

/*
6.5 Operator Instance

*/
    Operator startsWithSOp(
            "startsWithGst",
            startsWithSSpec.getStr(),
            startsWithSVM,
            Operator::SimpleSelect,
            startsWithSTM
    );

/*
The final steps are the same as for other operators.

*/





/*
5 Streams as Results of Operators (stream operators)

If a stream is the result of an operator, we call such an operator
stream-operator.  The main difference to other operators is in the
value mapping function.

We explain the implementation of a stream operator by the
operator ~getChars~. This operator gets a single string as
its argument and returns a stream of strings where each string
corresponds to a single character of the argument.

5.1 Type Mapping

The type mapping of a stream operator has no specials. The creation
of the result is a little bit more complicated as for simple types because
the typed stream must be returned.

*/
    ListExpr getCharsTM(ListExpr args) {
        // check number of arguments
        if (!nl->HasLength(args, 1)) {
            return listutils::typeError("wrong number of arguments");
        }
        // argument must be of type string
        if (!CcString::checkType(nl->First(args))) {
            return listutils::typeError("string expected");
        }
        // create the result type (stream string)
        return nl->TwoElemList(listutils::basicSymbol<Stream<CcString> >(),
                               listutils::basicSymbol<CcString>());
    }

/*
5.2 LocalInfo Class

The value mapping of a stream operator is called many times during the
execution of a query. We need a structure, storing the current state
of the operator. In the implementation of the ~getChars~ operator, we
have to store the current position within the input string.  We encapsulate
the state of the operator within a class and let do this class the
whole work.

*/
    class getCharsLI {
    public:
        // constructor: initializes the class from the string argument
        getCharsLI(CcString *arg) : input(""), pos(0) {
            if (arg->IsDefined()) {
                input = arg->GetValue();
            }
        }

        // destructor
        ~getCharsLI() {}

        // this function returns the next result or null if the input is
        // exhausted
        CcString *getNext() {
            if (pos >= input.length()) {
                return 0;
            }
            CcString *res = new CcString(true, input.substr(pos, 1));
            pos++;
            return res;
        }

    private:
        string input;  // input string
        size_t pos;    // current position
    };

/*
5.3 Value Mapping

The value mapping of stream operators has a lot of differences compared to
the value mapping of non-stream operator. One difference is
that the ~message~ argument must be used to select the action to do. The messages
are OPEN, REQUEST, and CLOSE. (if the operator supports progress estimation,
some more messages must be handled).
Furthermore, the ~local~ argument is used to store the current state of the
operator (and doing the computations). The ~addr~ pointer of ~local~ is
null at the first call of this operator. The operator is responsible to this
pointer. After receiving a close message, the pointer must be set to null.
Another difference to non-stream operators is that the result storage of
~s~ is not used. Instead, we write newly created objects into the ~addr~ pointer
of ~result~.

When an OPEN message is received, we firstly check whether  a
~localInfo~ is already stored by checking the ~addr~ unequal to null.
If so, we delete this structure and create a new one.
We set the ~addr~ pointer of the ~local~ argument to this structure. The result
of an OPEN message is always 0.

If a REQUEST message is received. We first look, whether we have already created
a local info. If not, we set the ~addr~ pointer of ~result~ to null. If there
is already such a structure, we compute the next result and store it into the
~addr~ pointer of ~result~. The computation of the next result is delegated to
the ~getNext~ function of the localInfo class. If there is a next result (addr
pointer of result is not null), the operator returns YIELD, otherwise CANCEL.

In the case of a CLOSE message, we free the memory allocated by the local info class
and set the ~addr~ pointer of ~local~ to null. The result to a CLOSE message is
always 0.

*/
    int getCharsVM(Word *args, Word &result, int message,
                   Word &local, Supplier s) {
        getCharsLI *li = (getCharsLI *) local.addr;
        switch (message) {
            case OPEN :
                if (li) {
                    delete li;
                }
                local.addr = new getCharsLI((CcString *) args[0].addr);
                return 0;
            case REQUEST:
                result.addr = li ? li->getNext() : 0;
                return result.addr ? YIELD : CANCEL;
            case CLOSE:
                if (li) {
                    delete li;
                    local.addr = 0;
                }
                return 0;
        }
        return 0;
    }

/*
5.4 Specification

The specification of a stream operator has no specials.

*/

    OperatorSpec getCharsSpec(
            " string -> stream(string)",
            " getCharsGst(_) ",
            " Seperates the characters of a string. ",
            " query  getCharsGst(\"secondo\") count"
    );

/*
5.5 Operator instance

The creation of the operator instance is the same as for non-stream operators.

*/
    Operator getCharsOp(
            "getCharsGst",
            getCharsSpec.getStr(),
            getCharsVM,
            Operator::SimpleSelect,
            getCharsTM
    );

/*
As usual, the final steps are:

  * add the operator to the algebra

  * define the syntax in the ~spec~ file

  * give an example in the ~examples~ file

  * test the operator in Secondo

*/





/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

    class KafkaAlgebra : public Algebra {
    public:
        KafkaAlgebra() : Algebra() {

            AddTypeConstructor(&SCircleTC);
            SCircleTC.AssociateKind(Kind::SIMPLE());

            AddOperator(&kafkaConsumerOp);
            AddOperator(&kafkaProducerOp);

            AddOperator(&perimeterOp);
            AddOperator(&startsWithSOp);
            AddOperator(&getCharsOp);
        }
    };

} // End namespace

extern "C"
Algebra *
InitializeKafkaAlgebra(NestedList *nlRef,
                       QueryProcessor *qpRef) {
    return new kafka::KafkaAlgebra;
}

