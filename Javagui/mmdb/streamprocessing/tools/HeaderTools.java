package mmdb.streamprocessing.tools;

import java.util.List;

import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;

public abstract class HeaderTools {

	public static void checkIdentifiersPresent(List<RelationHeaderItem> header,
			Class<? extends Node> caller, String... identifiers)
			throws TypeException {
		for (String identifier : identifiers) {
			HeaderTools.getHeaderItemForIdentifier(header, identifier, caller);
		}
	}

	public static void checkIdentifiersNotPresent(
			List<RelationHeaderItem> header, Class<? extends Node> caller,
			String... identifiers) throws TypeException {
		for (String identifier : identifiers) {
			boolean found = true;
			try {
				HeaderTools.getHeaderItemForIdentifier(header, identifier,
						caller);
			} catch (TypeException e) {
				found = false;
			}
			if (found) {
				throw new TypeException(
						"%s: Identifier \"%s\" should not be present!",
						caller.getSimpleName(), identifier);
			}
		}
	}

	public static void checkAttributeHasIFace(List<RelationHeaderItem> header,
			String identifier, Class<?> iFace, Class<? extends Node> caller)
			throws TypeException {
		RelationHeaderItem item = HeaderTools.getHeaderItemForIdentifier(
				header, identifier, caller);
		if (!iFace.isAssignableFrom(item.getType())) {
			throw new TypeException(
					"%s: Argument attribute \"%s\" has wrong type. Expected: %s. Actual: %s.",
					caller.getSimpleName(), identifier, iFace.getSimpleName(),
					MemoryAttribute.getTypeName(item.getType()));
		}
	}

	/**
	 * Returns the RelationHeaderItem representing the given identifier
	 * 
	 * @param header
	 *            The List of HeaderItems to search in
	 * @param identifier
	 *            The identifier to the HeaderItem look for
	 * @return The HeaderItem representing the given identifier
	 * @throws TypeException
	 *             If no HeaderItem is found for the given identifier
	 */
	public static RelationHeaderItem getHeaderItemForIdentifier(
			List<RelationHeaderItem> header, String identifier,
			Class<? extends Node> caller) throws TypeException {
		for (RelationHeaderItem item : header) {
			if (item.getIdentifier().equals(identifier)) {
				return item;
			}
		}
		throw new TypeException("%s: Identifier \"%s\" not present!",
				caller.getSimpleName(), identifier);
	}

	public static Class<? extends MemoryAttribute> getClassForHeaderItem(
			List<RelationHeaderItem> header, String identifier,
			Class<? extends Node> caller) throws TypeException {
		RelationHeaderItem item = HeaderTools.getHeaderItemForIdentifier(
				header, identifier, caller);
		return item.getType();
	}

	public static int getHeaderIndexForIdentifier(
			List<RelationHeaderItem> header, String identifier,
			Class<? extends Node> caller) throws TypeException {
		int headerIndex = -1;
		for (int i = 0; i < header.size(); i++) {
			RelationHeaderItem item = header.get(i);
			if (item.getIdentifier().equals(identifier)) {
				headerIndex = i;
				break;
			}
		}
		if (headerIndex == -1) {
			throw new TypeException("%s: Identifier \"%s\" not present!",
					caller, identifier);
		}
		return headerIndex;
	}

}
