package mmdb.streamprocessing.parser.nestedlist;

public class RealAtom extends Atom {

	private Float content;

	public RealAtom(float f) {
		this.content = f;
	}

	public Float getContent() {
		return content;
	}

	@Override
	public String printValueList() {
		return content.toString();
	}
	
	@Override
	public String printTypeList() {
		return "real";
	}

}
