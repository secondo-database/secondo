// What the query box offers while you type: the open database's own object
// names, the command words, and every operator the connected server has.
//
// Both name lists come from the server (App passes them down) -- the objects
// from `list objects`, which the left-hand panel already has, and the operators
// from `list operators`. Neither is a list maintained here: an operator is
// offered because the server it is talking to can run it, so enabling an
// algebra is enough to make its operators complete.

import type { CatalogObject, OperatorInfo } from "../api/client";

export interface Completion {
  /** What gets inserted. */
  text: string;
  /** Where it came from, shown greyed after the name -- the object's SECONDO
   *  type for a catalog entry, "command" or "operator" otherwise. */
  hint: string;
}

/** Commands, not operators: what a line can begin with. */
const KEYWORDS = [
  "query",
  "let",
  "delete",
  "open database",
  "close database",
  "create database",
  "list objects",
  "list databases",
  "list operators",
  "list algebras",
  "list types",
  "restore",
  "save",
  "select",
  "from",
  "where",
  "orderby",
  "groupby",
  "first",
  "create table",
  "drop table",
  "insert into",
  "update",
  "values",
];

/** What to offer before `/api/operators` has answered, or if it never does --
 *  the operators reached for most in day-to-day queries. A fallback only: the
 *  server's own list replaces it wholesale as soon as it arrives. */
export const FALLBACK_OPERATORS = [
  "feed",
  "consume",
  "filter",
  "project",
  "remove",
  "head",
  "count",
  "sortby",
  "groupby",
  "extend",
  "extract",
  "addid",
  "rename",
  "product",
  "symmjoin",
  "hashjoin",
  "sortmergejoin",
  "loopjoin",
  "windowintersects",
  "gettuples",
  "exactmatch",
  "range",
  "leftrange",
  "bbox",
  "inside",
  "intersects",
  "intersection",
  "union",
  "minus",
  "distance",
  "size",
  "no_components",
  "trajectory",
  "atinstant",
  "atperiods",
  "deftime",
  "rangevalues",
  "val",
  "inst",
  "present",
  "passes",
  "at",
  "initial",
  "final",
  "speed",
  "direction",
  "createbtree",
  "creatertree",
  "updatebyid",
  "deletebyid",
  "inserttuple",
];

/** The word being typed: letters, digits and underscores back from the caret.
 *  Returns "" when the caret is not in a word (after a space or a bracket), so
 *  nothing is offered until there is something to complete. */
export function tokenAt(textBeforeCaret: string): string {
  return /[A-Za-z_][A-Za-z0-9_]*$/.exec(textBeforeCaret)?.[0] ?? "";
}

/** The shortest token worth completing. One letter matches far too much to be
 *  a suggestion rather than a distraction. */
const MIN_TOKEN = 2;

/** A menu longer than this stops being a hint and starts being a panel over
 *  the map; narrowing the word is the better way to get to a long tail. */
const MAX_ITEMS = 10;

/**
 * What to offer for the word under the caret. Object names come first -- they
 * are the part a user cannot be expected to remember -- then commands, then
 * operators; within each, what starts with the token beats what merely
 * contains it, and shorter beats longer.
 *
 * Operators match on their *prefix* only. There are around 1800 of them, and a
 * substring match on a two-letter token pulls in a hundred names that have
 * nothing to do with what is being typed; an object name is worth guessing at
 * from the middle, an operator is not.
 */
export function completionsFor(
  textBeforeCaret: string,
  objects: CatalogObject[],
  operators: OperatorInfo[] = FALLBACK_OPERATORS.map((name) => ({
    name,
    syntax: "",
  }))
): Completion[] {
  const token = tokenAt(textBeforeCaret);
  if (token.length < MIN_TOKEN) return [];
  const needle = token.toLowerCase();

  const pools: { items: Completion[]; rank: number; prefixOnly?: boolean }[] = [
    {
      rank: 0,
      items: objects.map((o) => ({ text: o.name, hint: o.type })),
    },
    { rank: 1, items: KEYWORDS.map((k) => ({ text: k, hint: "command" })) },
    {
      rank: 2,
      prefixOnly: true,
      items: operators.map((o) => ({ text: o.name, hint: o.syntax || "operator" })),
    },
  ];

  const scored: { c: Completion; key: [number, number, number, string] }[] = [];
  for (const { items, rank, prefixOnly } of pools) {
    for (const c of items) {
      const at = c.text.toLowerCase().indexOf(needle);
      if (at < 0 || (prefixOnly && at !== 0)) continue;
      // A name typed out in full stays on the list. It has nothing left to
      // insert, but its presence is the answer to "is this actually an object
      // here, and did I spell it right?" -- the same reason a shell keeps
      // showing a completion that is already complete.
      scored.push({ c, key: [at === 0 ? 0 : 1, rank, c.text.length, c.text] });
    }
  }

  scored.sort((a, b) => {
    for (let i = 0; i < 3; i++) {
      const d = (a.key[i] as number) - (b.key[i] as number);
      if (d) return d;
    }
    return String(a.key[3]).localeCompare(String(b.key[3]));
  });

  return scored.slice(0, MAX_ITEMS).map((s) => s.c);
}

/** The text and caret position after accepting `choice`: the token under the
 *  caret is replaced, everything else is left exactly as it was. */
export function applyCompletion(
  value: string,
  caret: number,
  choice: Completion
): { value: string; caret: number } {
  const token = tokenAt(value.slice(0, caret));
  const start = caret - token.length;
  return {
    value: value.slice(0, start) + choice.text + value.slice(caret),
    caret: start + choice.text.length,
  };
}
