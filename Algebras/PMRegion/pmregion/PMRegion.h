/* 
  This file is part of libpmregion
  
  File:   PMRegion.h
  Author: Florian Heinz <fh@sysv.de>
  
  1 PMRegion.h
    Interface class Definitions for PMregion and RList
 
*/

#ifndef PMREGION_H
#define PMREGION_H

namespace pmr {

class RList;
class MReal;
class MBool;

class PMRegion {
	public:
		PMRegion() {}
		~PMRegion() {}
		PMRegion(Polyhedron p) {
			polyhedron = p;
		}
		PMRegion operator+(PMRegion& pmr);
		PMRegion operator-(PMRegion& pmr);
		PMRegion operator*(PMRegion& pmr);
		RList atinstant(double instant);
		MBool mpointinside(RList& mpoint);
		MBool intersects(PMRegion& pmr);
		MReal perimeter();
		MReal area();
		RList traversedarea();

		static PMRegion fromRList(RList rl);
		RList toRList();

		static PMRegion fromOFF(std::string off);
		string toOFF();

		static PMRegion fromMRegion(RList mr);
		RList toMRegion();

	protected:
		Polyhedron polyhedron;
};

class RList {
	protected:
		int type;
		string str;
		double nr;
		bool boolean;
		string ToString(int indent);

	public:
		vector<RList> items;

		RList();
		void append(double nr);
		void append(string str);
		void appendsym(string str);
		void append(bool val);
		void append(RList l);
		void prepend(RList l);
		RList* point(double x, double y);
		void concat(RList l);
		double getNr () {
			assert(type == NL_DOUBLE);
			return nr;
		}
		bool getBool () {
			assert(type == NL_BOOL);
			return boolean;
		}
		string getString () {
			assert(type == NL_STRING);
			return str;
		}
		string getSym () {
			assert(type == NL_SYM);
			return str;
		}
		int getType () {
			return type;
		}
		RList* nest();

		RList obj(string name, string type);
		static RList parse(std::istream& f);
		string ToString();
};

class MReal {
	public:
		RList rl;

		MReal() {
			rl.appendsym("OBJECT");
			rl.appendsym("mreal");
			RList empty;
			rl.append(empty);	
			rl.appendsym("mreal");
			rl.append(empty);
		}

		void append (Kernel::FT start, Kernel::FT end,
				Kernel::FT a, Kernel::FT b, Kernel::FT c) {
			RList ureal;
			RList interval;
			interval.append(timestr(start));
			interval.append(timestr(end));
			interval.append((bool)true);
			interval.append((bool)false);

			RList coeffs;
			coeffs.append(::CGAL::to_double(a));
			coeffs.append(::CGAL::to_double(b));
			coeffs.append(::CGAL::to_double(c));
			coeffs.append((bool)false);

			ureal.append(interval);
			ureal.append(coeffs);
			rl.items[4].append(ureal);
		}
};

class MBool {
	public:
		RList rl;

		MBool() {
			rl.appendsym("OBJECT");
			rl.appendsym("mbool");
			RList empty;
			rl.append(empty);	
			rl.appendsym("mbool");
			rl.append(empty);
		}

		void append (Kernel::FT start, Kernel::FT end, bool value) {
			RList ubool;
			RList interval;
			interval.append(timestr(start));
			interval.append(timestr(end));
			interval.append((bool)true);
			interval.append((bool)false);

		ubool.append(interval);
		ubool.append(value);
		rl.items[4].append(ubool);
	}
};

}

#endif /* PMREGION_H */
