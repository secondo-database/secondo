package mmdb.streamprocessing.tools;

import java.util.List;

import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;

/**
 * Tool class for typecheck operations on Relation- or TupleHeaders.
 * 
 * @author Bjoern Clasen
 */
public abstract class HeaderTools {

	/**
	 * Checks if a list of code identifiers is present in the given header.
	 * 
	 * @param header
	 *            the header that should contain the identifiers.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param identifiers
	 *            list of identifiers that should be present.
	 * @throws TypeException
	 *             if any identifier is not present.
	 */
	public static void checkIdentifiersPresent(List<RelationHeaderItem> header,
			Class<? extends Node> caller, String... identifiers)
			throws TypeException {
		for (String identifier : identifiers) {
			HeaderTools.getHeaderItemForIdentifier(header, identifier, caller);
		}
	}

	/**
	 * Checks if none of the given identifiers are present in the header.
	 * 
	 * @param header
	 *            the header that should NOT contain the identifiers.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param identifiers
	 *            list of identifiers that should NOT be present.
	 * @throws TypeException
	 *             if any of the given identifiers ARE present.
	 */
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

	/**
	 * Checks if the MemoryAttribute linked to the given identifier in the
	 * header implements the given interface.
	 * 
	 * @param header
	 *            the header to extract the attribute type from.
	 * @param identifier
	 *            the identifier which is linked to the attribute type.
	 * @param iFace
	 *            the interface that the attribute type should implement.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @throws TypeException
	 *             if the attribute type does not implement the given interface
	 */
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
	 * Returns the RelationHeaderItem that contains the given identifier.
	 * 
	 * @param header
	 *            The header to search in
	 * @param identifier
	 *            The identifier to look for in the header
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @return The HeaderItem that contains the given identifier
	 * @throws TypeException
	 *             If the header does not contain the given identifier
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

	/**
	 * Returns the type of the MemoryAttribute linked to the given identifier.
	 * 
	 * @param header
	 *            the header that should contain the given identifier.
	 * @param identifier
	 *            the identifier the looked for attribute is linked to.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @return the class of the MemoryAttribute
	 * @throws TypeException
	 *             if the given identifier is not present in the header.
	 */
	public static Class<? extends MemoryAttribute> getClassForHeaderItem(
			List<RelationHeaderItem> header, String identifier,
			Class<? extends Node> caller) throws TypeException {
		RelationHeaderItem item = HeaderTools.getHeaderItemForIdentifier(
				header, identifier, caller);
		return item.getType();
	}

	/**
	 * Returns the index (position) of the given identifier in the header.
	 * 
	 * @param header
	 *            the header that should contain the identifier.
	 * @param identifier
	 *            the identifier whose index is looked for.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @return the index of the given identifier as {@code int}.
	 * @throws TypeException
	 *             if the header does not contain the given identifier.
	 */
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
