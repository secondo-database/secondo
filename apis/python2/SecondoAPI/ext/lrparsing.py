# Lrparsing.py is a LR(1) parser hiding behind a pythonic interface.  It takes
# as input a grammar and a string to be parsed, and outputs the parse tree.
#
# Copyright (c) 2013,2014,2015,2016,2017,2018 Russell Stuart.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# The copyright holders grant you an additional permission under Section 7
# of the GNU Affero General Public License, version 3, exempting you from
# the requirement in Section 6 of the GNU General Public License, version 3,
# to accompany Corresponding Source with Installation Information for the
# Program or any work based on the Program. You are still required to
# comply with all other Section 6 requirements to provide Corresponding
# Source.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import ast
import collections
import hashlib
import itertools
import re
import string
import sys


#
# Python2/3 compatibility hacks.
#
# Written this odd way to preserve 100% test coverage.
#
StandardError = Exception if sys.version_info >= (3,) else StandardError
python3_metaclass = (lambda cls: cls) if sys.version_info < (3,) else (
    lambda cls: cls.__metaclass__(
        cls.__name__, cls.__bases__, dict(cls.__dict__)))
string_types = basestring if sys.version_info < (3,) else str
string_maketrans = (
    string.maketrans if sys.version_info < (3,) else str.maketrans)
to_str = lambda s: s if isinstance(s, string_types) else str(s)


#
# Common base class for all exceptions here.
#
class LrParsingError(StandardError):
    pass


#
# Raised if the Grammar isn't valid.
#
class GrammarError(LrParsingError):
    pass


#
# Base class for errors raised at parsing time.
#
class ParsingError(LrParsingError):
    pass


#
# Raised if an invalid syntax is given.
#
class TokenError(ParsingError):

    def __init__(self, message, data, offset, line, column):
        self.data = data
        self.offset = offset
        self.line = line
        self.column = column
        super(TokenError, self).__init__(message, data, offset, line, column)


#
# Raised if an invalid syntax is given.
#
class ParseError(ParsingError):
    input_token = None
    stack = None

    def __init__(self, input_token, stack):
        def comma_or(lst):
            strs = sorted("%s" % l for l in lst)
            if len(strs) == 1:
                return strs[0]
            return ', '.join(strs[:-1]) + ' or ' + strs[-1]
        self.input_token = input_token
        self.stack = stack
        lr1_state = stack[-1][0]
        if len(lr1_state.actions) >= 10:
            msg = "Got unexpected %s" % (input_token[0],)
        else:
            msg = "Got %s when expecting %s" % (
                input_token[0], comma_or(lr1_state.actions))
        if len(lr1_state.rules) < 10:
            msg += " while trying to match %s" % (comma_or(lr1_state.rules))
        msg += " in state %d" % (lr1_state.id,)
        position = input_token[0].position(input_token)
        if position:
            msg = position + ": " + msg
        super(ParseError, self).__init__(msg)


#
# Print a set of symbols.
#
def str_symbol_set(symbol_set):
    return '[%s]' % (','.join(sorted(str(symbol) for symbol in symbol_set)),)


#
# An LR(0) Item.  An item is just a production and the position the parser
# is up to in parsing it - the dot_pos.  Eg:
#
#    sym0 sym1 . sym2
#
# means the parser is processing a production wants to see the symbols
# "sym0 sym1 sym2" in order, and it has seen sym1 and sym2.
#
class Lr0Item(object):
    __slots__ = ('dot_pos', '_key', 'lr0_item', 'production')

    def __new__(cls, production, dot_pos, cache):
        lr0_item = (production, dot_pos)
        result = cache.get(lr0_item, None)
        if result is None:
            result = super(Lr0Item, cls).__new__(cls)
            result.lr0_item = lr0_item
            result.dot_pos = dot_pos
            result.production = production
            result._key = (
                (str(result.production.lhs),) +
                tuple(str(sym) for sym in result.production.rhs) +
                (result.dot_pos,)
            )
            cache[result.lr0_item] = result
        return result

    def __repr__(self):
        rhs = self.production.rhs
        ll = lambda s, e: [str(symbol) for symbol in rhs[s:e]]
        prod = ll(0, self.dot_pos) + ['^'] + ll(self.dot_pos, len(rhs))
        return "%s = %s" % (self.production.lhs, ' '.join(prod))

    def key(cls, lr0_item):
        return lr0_item._key
    key = classmethod(key)


#
# An item in an LR(1) grammar.  It is just a LR(0) item, together with the
# set a of tokens that could follow the production called the lookahead.
# Eg, given the Item():
#
#   L ::= sym0 sym1 sym2 .  [tokA, tokB]
#
# The parser has seen all the symbols in this production, so if the next token
# to be processed is tokA or tokB, the production can be reduced (ie replaced)
# with it's left hand side, ie L
#
class Lr1Item(object):
    __slots__ = ('dot_pos', 'lookahead', 'lr0_item', 'production')

    def __init__(self, lr0_item, lookahead):
        self.lr0_item = lr0_item
        self.production = self.lr0_item.production
        self.dot_pos = self.lr0_item.dot_pos
        self.lookahead = lookahead

    def lr1_shift(self, cache):
        lr0_item = Lr0Item(self.production, self.dot_pos + 1, cache)
        return Lr1Item(lr0_item, set(self.lookahead))

    def __repr__(self):
        return "%r %s" % (self.lr0_item, str_symbol_set(self.lookahead),)

    def sorted(cls, iterable):
        return sorted(iterable, key=lambda item: Lr0Item.key(item.lr0_item))
    sorted = classmethod(sorted)


#
# An Lr0Kernel.  A "kernel" is the state of the parser.  It is just a set of
# items (Lr0Items in the case of an Lr0Kernels).  The initial kernel is just
# the grammar's start production, eg:
#
#   G ::= . E <eoi>
#
# Successive kernels are generated by looking every production that can
# be reached from a previous kernel if a particular given token is seen.
# For example, if the rest of the grammar is:
#
#   E ::= E / E
#   E ::= n
#
# Then after seeing an n, the Lr0Kernel would be:
#
#   E ::= n .
#
class Lr0Kernel(object):
    __slots__ = ("lr0_items",)

    def __new__(cls, lr1_items, cache):
        lr0_items = frozenset(item.lr0_item for item in lr1_items)
        result = cache.get(lr0_items, None)
        if result is None:
            result = super(Lr0Kernel, cls).__new__(cls)
            result.lr0_items = lr0_items
            cache[result.lr0_items] = result
        return result


#
# An Lr1State is the compiled version of an ItemSet.  Ie, all the
# information the Parser doesn't need has been discarded.
#
class Lr1State(int):
    if sys.version_info < (3,):
        __slots__ = ('actions', 'gotos', 'id', 'rules')

    def __new__(cls, id, actions, gotos, rules):
        result = super(Lr1State, cls).__new__(cls, id)
        result.actions = actions
        result.gotos = gotos
        result.id = id
        result.rules = rules
        return result

    def __repr__(self):
        def p(act):
            if len(act) == 1:
                return "shift %d" % act
            if len(act) == 2 or act[2] is None:
                return "reduce %d %d" % act[:2]
            return "reduce %d %d %s" % act[:3]
        result = [str(self)]
        if self.actions:
            result.append("  -- actions")
            for token in sorted(self.actions, key=lambda sym: str(sym)):
                result.append("    %s: %s" % (token, p(self.actions[token],)))
        if self.gotos:
            result.append("  -- gotos")
            for nonterm_number in sorted(self.gotos, key=lambda sym: str(sym)):
                result.append(
                    "    %s: %s" %
                    (nonterm_number, self.gotos[nonterm_number],))
        return '\n'.join(result)

    def __str__(self):
        return "Lr1State:%d" % (int(self),)

    def to_flat(self, grammar):
        actions = {}
        for token, action in self.actions.items():
            if len(action) == 1:
                new_action = action[0]
            elif action[2] is None:
                new_action = tuple(action[:2])
            else:
                new_action = (action[0], action[1], action[2].name)
            actions[token.name] = new_action
        rules = sorted(rule.name for rule in self.rules)
        return (actions, self.gotos, rules)
    PYTHON_VAR_RE = re.compile(
        "(?i)^[a-z_][a-z_0-9]*(?:[.][a-z_][a-z_0-9]*)*$")

    def from_flat(cls, index, flat, rules, token_registry):
        actions = {}
        for token_name, action in flat[0].items():
            if not isinstance(action, tuple):
                new_action = (action,)
            elif len(action) == 2:
                new_action = (action[0], action[1], None, None)
            else:
                new_action = (action[0], action[1], rules[action[2]], None)
            token = token_registry[token_name]
            actions[token] = new_action
        target_rules = (rules[rule_name] for rule_name in flat[2])
        return cls(index, actions, flat[1], set(target_rules))
    from_flat = classmethod(from_flat)

    def sorted(cls, table):
        return sorted(table)
    sorted = classmethod(sorted)


#
# An ItemSet is a state in the LR(1) grammar.  An ItemSet is identical in
# concept to an LR(0) kernel, but it consists of Lr1Item's rather than
# Lr0Item's.  In other words, the items contain the tokens they expect to
# follow the production.  These tokens are shown in []'s.
#
# Following on from the example above:
#
#   G ::= . E <eoi>, [__empty__]
#
# And if an n is accepted:
#
#   E ::= n .,  [<eoi>, /]
#
class ItemSet(object):
    ID = 0
    __slots__ = (
        'actions', '_closure', '_goto_cache', 'gotos', 'id',
        '_kernel', '_lhs_prio', 'lr0_kernel', 'prio',)

    def __init__(self, items, cache):
        items = tuple(items)
        self._kernel = dict((item[0].lr0_item, item[0]) for item in items)
        self.prio = dict((item[0].lr0_item, item[1]) for item in items)
        self.lr0_kernel = Lr0Kernel(iter(self), cache)
        self.actions = None
        self.gotos = {}
        self._closure = None
        self._goto_cache = None
        self._lhs_prio = None
        self.id = self.ID
        self.__class__.ID += 1

    def __str__(self):
        return "ItemSet:%d" % (self.id,)

    def __repr__(self):
        result = [str(self)]
        for kernel_item in Lr1Item.sorted(iter(self)):
            line = "  %r %s" % (kernel_item, self.repr_prio(kernel_item))
            result.append(line)
        if self._closure:
            result.append("  -- closure")
            for closure_item in Lr1Item.sorted(self._closure.values()):
                line = "    %r %s" % (
                    closure_item, self.repr_prio(closure_item))
                result.append(line)
        if self.actions:
            result.append("  -- actions")
            for token in sorted(self.actions, key=lambda action: str(action)):
                actions = self.actions[token]
                if isinstance(actions, Action):
                    lst = repr(actions)
                else:
                    lst = ', '.join(sorted(repr(act) for act in actions))
                result.append("    %s: %s" % (token, lst,))
        if self.gotos:
            result.append("  -- gotos")
            for symbol in sorted(self.gotos, key=lambda symbol: str(symbol)):
                result.append("    %s: %s" % (symbol, self.gotos[symbol],))
        return '\n'.join(result)

    def repr_prio(self, item):
        result = []
        for prio, lookahead in sorted(self.prio[item.lr0_item].items()):
            result.append("%r:%s" % (prio, str_symbol_set(lookahead),))
        return '{%s}' % ', '.join(result)

    #
    # Return all lhs tokens we generate.
    #
    def rules(self):
        return set(item.production.lhs.get_rule() for item in self)
    rules = property(rules)

    #
    # Compute the closure for the passed items.  Ie, given the item
    # {A ::=  a . B c}, add {B ::= . C d} and repeat for B.  In other words,
    # the kernel plus its closure contains every production we can be
    # expanding, and the position in them.
    #
    # X ::= a ^ b c, [l]
    #
    # b ::= ^ f g, [follow(c)]
    #
    def _close_kernel_items(self, items, cache):
        modified = False
        queue = collections.deque(items)
        empty_token = cache['__empty__']
        while queue:
            #
            # Find the next symbol that will be consumed by item.
            #
            item = queue.popleft()
            rhs, dot_pos = item.production.rhs, item.dot_pos
            if dot_pos >= len(rhs):
                continue
            symbol = rhs[dot_pos]
            if isinstance(symbol, TokenSymbol):
                continue
            #
            # Find all tokens we could possibly see after consuming that
            # symbol.
            #
            first_set = self.symbol_seq_first_set(rhs[dot_pos + 1:], cache)
            if empty_token not in first_set:
                lookahead = set(first_set)
                no_empty = first_set
            else:
                no_empty = first_set - empty_token.first_set
                lookahead = no_empty | item.lookahead
            #
            # If the next symbol is a nonterm, then add all of its productions
            # to the closure.
            #
            for production in symbol.productions:
                lr0_item = Lr0Item(production, 0, cache)
                existing = self._closure.get(lr0_item, None)
                if existing is not None:
                    extra = lookahead - existing.lookahead
                    existing.lookahead |= extra
                    closure = existing
                else:
                    extra = lookahead
                    closure = Lr1Item(lr0_item, lookahead)
                    self._closure[lr0_item] = closure
                    self.prio[closure.lr0_item] = {}
                if not extra:
                    continue
                queue.append(closure)
                modified = True
                #
                # Push the priority through the closure.  In other words, if
                # the rhs of this production assigned this lhs a priority,
                # then add this priority to the list we inherited from the
                # ItemSet's that goto us.  Eg, given:
                #
                #   START = Prio(b,c) __end_of_input__, b.prio==0 and c.prio==1
                #   b     = Prio(d+'X', e+'Y'), so d.prio==0 and e.prio==1
                #   c     = Prio(e+'X', d+'Y'), so d.prio==1 and e.prio==0
                #   d     = 'T'
                #   e     = 'T'
                #
                # The prio of START is always (), so b's prio will be (0,)
                # ie "() + (0,)", and c's prio will be (1,) and after that
                # we expect both to be followed by __end_of_input__.  From b's
                # production d's prio will be b's plus 0, ie
                # "(0,) + (0,) = (0,0,)", but only when followed by a 'X' (ie
                # the lookahead is 'X').  From c's production d's prio will be
                # (1,1), but only when followed by a 'Y'.
                #
                # Thus for 'd' we end up with these priorities:
                #
                #    { (0,0,): set('X'), (0,1,): set('Y') }
                #
                # and for 'e' we end up with:
                #
                #    { (1,0): set('X'), (1,1): set('Y') }
                #
                # If the resulting parser is given the input string:
                #
                #   'T' 'Y' __end_of_input__.
                #
                # First it has to decide whether 'T' is a 'd' or an 'e' as
                # both will resolve to 'T'.  A normal LR(1) would be stuck
                # with a reduce/reduce conflict, but since the d's priority
                # for 'Y' (0,1) < e's priority (1,1), we chose 'd'.  We don't
                # do that choosing here, but we create the priorities so it
                # can happen should a conflict arise.
                #
                rank = closure.production.lhs.rank
                priority = closure.production.lhs.priority
                if priority is not None:
                    rank -= 1
                append = (0,) * max(0, rank - item.production.lhs.rank)
                if priority is not None:
                    append += (priority,)
                prio_items = self.prio[item.lr0_item].items()
                for itemset_prio, item_prio in prio_items:
                    existing_sets = self.prio[closure.lr0_item]
                    if empty_token not in first_set:
                        add = no_empty
                    else:
                        add = no_empty | extra & item_prio
                    existing_prio = itemset_prio[:rank] + append
                    if existing_prio not in existing_sets:
                        existing_sets[existing_prio] = add
                    else:
                        existing_sets[existing_prio] |= add
        return modified

    #
    # Compute the first_set for a sequence of Symbols.  If the sequence is:
    #
    #   sym0 sym1 sym2
    #
    # The first_set is the same as the first_set of sym0, and if the sym0
    # can be empty then the first_set of sym1 and so on.
    #
    def symbol_seq_first_set(cls, symbol_sequence, cache):
        syms = tuple(symbol_sequence)
        result = cache.get(syms, None)
        if result is None:
            empty_token = cache['__empty__']
            first_set = set(empty_token.first_set)
            for symbol in syms:
                first_set |= symbol.first_set
                if empty_token not in symbol.first_set:
                    first_set -= empty_token.first_set
                    break
            result = frozenset(first_set)
            cache[syms] = result
        return result
    symbol_seq_first_set = classmethod(symbol_seq_first_set)

    #
    # Iterating over us returns the Lr1Item's in our kernel.
    #
    def __iter__(self):
        return iter(self._kernel.values())

    #
    # Return a generator for the kernel + closure.
    #
    def all_items(self):
        return itertools.chain(self, self._closure.values())

    #
    # Compute the closure for ourselves.
    #
    def compute_closure(self, cache):
        if self._closure is None:
            empty_token = cache['__empty__']
            for lr0_item, old_prio_dict in self.prio.items():
                prio_dict = {}
                last_lookahead = empty_token.first_set
                for prio, lookahead in sorted(old_prio_dict.items()):
                    if lookahead != last_lookahead:
                        prio_dict[prio] = lookahead
                        last_lookahead = lookahead
                self.prio[lr0_item] = prio_dict
            self._closure = {}
            self._close_kernel_items(iter(self), cache)

    #
    # Calculate the kernel of the goto set, given a particular symbol.
    #
    def goto_sets(self, cache):
        if self._goto_cache is None:
            dot_symbols = collections.defaultdict(set)
            for item in self.all_items():
                dot_pos, rhs = item.dot_pos, item.production.rhs
                if dot_pos < len(rhs):
                    dot_symbols[rhs[dot_pos]].add(item)
            self._goto_cache = {}
            for symbol in sorted(dot_symbols, key=lambda symbol: symbol.id):
                gen = (
                    (item.lr1_shift(cache), self.prio[item.lr0_item])
                    for item in dot_symbols[symbol])
                item_set = ItemSet(gen, cache)
                self._goto_cache[symbol] = item_set
        return self._goto_cache

    #
    # Check for reduce/reduce compatibility between ItemSet's.  The two
    # ItemSet's passed must have the same same lr0 kernel.  ItemSet's with
    # different lr0 kernels are never compatible.
    #
    # When to reduce and what to is determined by the lookahead:
    #
    #    S ::= W;  S ::= X;  S ::= Y;  S ::= Z
    #    W ::= a P i
    #    X ::= a Q j
    #    Y ::= b P j
    #    Z ::= b Q i
    #    P ::= c
    #    Q ::= c
    #
    # Produces these ItemSets, among others:
    #
    #    WX:   P -> c ., [i]  (i-->reduce(P)); Q -> c ., [j]  (j-->reduce(Q))
    #    YZ:   P -> c ., [j]  (j-->reduce(Q)); Q -> c ., [i]  (i-->reduce(P))
    #
    # In this case merging WX and YZ would produce reduce conflicts because if
    # we see the token i, we can't both reduce(P) and reduce(Q).
    #
    def compatible(self, other):
        #
        # Compatible() isn't always called, so lazily evaluate
        # ItemSet._lhs_prio.
        #
        def lhs_prio(item_set):
            if item_set._lhs_prio is None:
                all_lookaheads = None
                lhs_prio = {}
                for lr0_item in item_set._kernel:
                    for prio, lookahead in item_set.prio[lr0_item].items():
                        key = (lr0_item.production.lhs, prio)
                        if key not in lhs_prio:
                            lhs_prio[key] = lookahead
                        else:
                            lhs_prio[key] |= lookahead
                        if all_lookaheads is None:
                            all_lookaheads = lookahead
                        else:
                            all_lookaheads |= lookahead
                item_set._lhs_prio = (lhs_prio, all_lookaheads)
            return item_set._lhs_prio
        if self is other:
            return True
        #
        # For an LR(1) scheme we are saying for a given lhs in self its
        # lookahead set can only share lookahead tokens with the same lhs in
        # other.
        #
        # For us it gets a little more complex, as we are carrying
        # ItemSet.prio's and they have to be compatible as well.  However,
        # turns out this reduces to insisting that (lhs, prio) combinations
        # can't share lookaheads suffices.
        #
        self_lhs_prio, self_all_lookaheads = lhs_prio(self)
        other_lhs_prio, other_all_lookaheads = lhs_prio(other)
        common = self_all_lookaheads & other_all_lookaheads
        for key, self_lookaheads in self_lhs_prio.items():
            other_lookaheads = other_lhs_prio.get(key, None)
            if other_lookaheads is not None:
                if (self_lookaheads & common) != (other_lookaheads & common):
                    return False
        return True

    #
    # Merge two ItemSet's.  They must be ItemSet.compatible().  Return True
    # if the merge altered 'self', so it needs a new closure computed.
    #
    def merge(self, other, cache):
        closure_items = []
        for item in self:
            other_item = other._kernel[item.lr0_item]
            modified = False
            item_prio = self.prio[item.lr0_item]
            expanded = False
            for prio, other_lookahead in other.prio[item.lr0_item].items():
                self_lookahead = self.prio[item.lr0_item].get(prio, None)
                if self_lookahead is None:
                    expanded = expanded or item_prio.copy()
                    expanded[prio] = other_lookahead
                elif not self_lookahead >= other_lookahead:
                    expanded = expanded or item_prio.copy()
                    expanded[prio] |= other_lookahead
                if expanded:
                    self.prio[item.lr0_item] = expanded
                    modified = True
            if modified:
                item.lookahead |= other_item.lookahead
                closure_items.append(item)
        if not closure_items:
            return False
        self._goto_cache = None
        self._lhs_prio = None
        self._close_kernel_items(closure_items, cache)
        return True

    def sorted(cls, table):
        return sorted(table, key=lambda item_set: item_set.id)
    sorted = classmethod(sorted)


#
# LR parser actions.  These have to be tuples because that is what the
# lr1_parser expects in the optimised case.  These are for the non-optimised
# case, so we carry more information for debugging and priority resolution.
# Nonetheless these must be backwards compatible with what
# Grammar.optimise_parsing_table() produces.
#
class Action(tuple):
    __slots__ = ()

    #
    # Actions for an owning item are placed into a set.  Identical actions
    # can't be in that set, so __hash__ and __eq__ must be implemented
    # accordingly.
    #
    def __hash__(self):
        raise NotImplementedError()

    def __eq__(self, other):
        raise NotImplementedError()

    #
    # Sort order.  Only important for tests where we need a repeatable test
    # outcomes.
    #
    def __lt__(self, other):
        return self.key() < other.key()

    #
    # This function returns three things:
    #
    #   (lhs, low, high)
    #
    # where:
    #
    #    lhs    Is the symbol we will reduce to, or None if there could be
    #           several.  This is used to compare associativity.
    #
    #    low    The lowest priority for the passed token.
    #
    #    high   The highest priority for the passed token.
    def precedence(self, token, item_set):
        raise NotImplementedError()


#
# A Shift action - consume a token, ie move it onto the stack.
#
class ShiftAction(Action):
    __slots__ = ()

    def __new__(cls, *next_state):
        result = Action.__new__(cls, next_state)
        return result

    def __repr__(self):
        return "shift %s" % self[0]

    def __hash__(self):
        return hash(self[0])

    def __eq__(self, other):
        return isinstance(other, ShiftAction) and self[0] == other[0]

    #
    # Key used for sorting.  Used only to get repeatable tests.
    #
    def key(self):
        return "shift", self[0].id

    #
    # In a shift:
    #
    #   lhs     Is the lhs of all items if they are the same, otherwise None.
    #
    #   low     The lowest prio for the token in the kernel.
    #
    #   high    The highest prio for the token in the kernel.
    #
    def precedence(self, token, item_set):
        my_item_set = self[0]
        lhs = next(iter(my_item_set)).production.lhs
        low = (1e100,)
        high = ()
        for lr1_item in my_item_set:
            if lhs != lr1_item.production.lhs:
                lhs = None
            items = my_item_set.prio[lr1_item.lr0_item].items()
            for prio, lookahead in items:
                if token in lookahead:
                    if low > prio:
                        low = prio
                    if high < prio:
                        high = prio
        return lhs, low, high


#
# A reduce action - ie the top of the stack is a production we recognise.
# Replace it with it's lhs.
#
class ReduceAction(Action):
    __slots__ = ()

    def __new__(cls, lr1_item):
        lhs = lr1_item.production.lhs
        output = lhs if isinstance(lhs, Rule) and lhs.name[0] != '_' else None
        me = (lhs, len(lr1_item.production.rhs), output, lr1_item)
        result = Action.__new__(cls, me)
        return result

    def __repr__(self):
        return "reduce %s = %s" % (
            self[0], ' '.join(str(sym) for sym in self[3].production.rhs))

    def __hash__(self):
        return hash(self[:3])

    def __eq__(self, other):
        return isinstance(other, ReduceAction) and self[:3] == other[:3]

    #
    # Key used for sorting.  Used only to get repeatable tests.
    #
    def key(self):
        return "reduce", self[0].id, self[3].dot_pos

    #
    # In a reduce:
    #
    #   lhs     Is the lhs of the target production.
    #
    #   low     The lowest prio for the token for this Lr1Item.
    #
    #   high    The highest prio for the token for this Lr1Item.
    #
    def precedence(self, token, item_set):
        lr1_item = self[3]
        lhs = lr1_item.production.lhs
        low = (1e100,)
        high = ()
        for prio, lookahead in item_set.prio[lr1_item.lr0_item].items():
            if token in lookahead:
                if low > prio:
                    low = prio
                if high < prio:
                    high = prio
        return lhs, low, high


#
# The thing that implements the grammar - the Parser.
#
class Parser(object):
    VERSION = "0.1"
    comments = None         # object,  TokenSymbol(), list of tokens.
    empty_token = None      # object,  MetaToken("__empty__")
    eoi_token = None        # object,  MetaToken("__end_of_input__")
    epoch_symbol = None     # object,  Rule(), <G> = START __end_of_input__
    parser_name = None      # string,  Name of the parser
    parsing_table = None    # tuple,   (Lr1State(), ...)
    rules = None            # dict,    {"name": Rule(), ...}
    token_registry = None   # object,  TokenRegistry()
    unused_symbols = None   # set,     SymbolSet(Rule(), ...)
    whitespace = None       # string,  Characters defined to be whitespace

    def __init__(self, parser_name, dct):
        def new_meta(name):
            meta = MetaToken(name)
            return meta.resolve_symbol(name, self.rules, self.token_registry)
        #
        # Step 1 is to replace all productions with a Rule() equivalent and
        # find the TokenRegistry.
        #
        self.parser_name = parser_name
        rule_symbols, self.rules, token_registry = (
            self.catalogue_symbols(dct))
        if token_registry is None:
            self.token_registry = TokenRegistry()
        else:
            token_registry.restore_dicts()
            self.token_registry = token_registry()
        for name in sorted(self.rules):
            self.resolve_rule(self.rules[name], rule_symbols)
        self.empty_token = new_meta("__empty__")
        self.eoi_token = new_meta("__end_of_input__")
        for name in sorted(self.rules):
            self.resolve_symbol(self.rules[name])
            if name in self.token_registry:
                msg = "A token and symbol share the same name %r"
                raise GrammarError(msg % name)
        #
        # Create the starting production for the grammar.
        #
        start_symbol = dct.get("START", None)
        if start_symbol is None:
            raise GrammarError("No START symbol defined")
        if not isinstance(start_symbol, Rule):
            raise GrammarError("START is not a Nonterm")
        if len(start_symbol.dict) != 0:
            raise GrammarError("START symbol may not have dictionary elements")
        epoch_symbol = Sequence()
        epoch_symbol.nested = [start_symbol, self.eoi_token]
        self.epoch_symbol = Rule('<%s>' % self.parser_name, epoch_symbol)
        epoch_symbol.parent = self.epoch_symbol
        if self.epoch_symbol.name in self.rules:
            msg = "Symbol name %r is reserved" % self.epoch_symbol.name
            raise GrammarError(msg)
        self.rules[self.epoch_symbol.name] = self.epoch_symbol
        for rule in self.rules.values():
            rule.resolved = True
        #
        # Get the special cased tokens.
        #
        self.whitespace = dct.get("WHITESPACE", None)
        self.token_registry.compile_tokens(self.whitespace)
        comments = dct.get("COMMENTS", None)
        if comments is None:
            self.comment_tokens = None
        else:
            error = not isinstance(comments, Rule)
            if not error:
                comment_symbol = comments.nested[0]
                if isinstance(comment_symbol, TokenSymbol):
                    self.comment_tokens = comments.nested
                elif isinstance(comment_symbol, Choice):
                    error = all(
                        not isinstance(sym, TokenSymbol)
                        for sym in comments.nested[0].nested)
                    if not error:
                        self.comment_tokens = comments.nested[0].nested
            if error:
                raise GrammarError("COMMENTS must be Token | Token ...")
            del self.rules['COMMENTS']

    #
    # Compile the grammar into an LR(1) parse_table, or raise a GrammarError
    # if there is a problem with the grammar.
    #
    def compile_grammar(self):
        if self.parsing_table:
            return
        #
        # Initialise the grammar by creating the start production.  Then
        # compile it.
        #
        used_symbols = self.epoch_symbol.compile_grammar(self.empty_token)
        self.unused_symbols = frozenset(
            symbol for symbol in self.rules.values()
            if symbol not in used_symbols)
        #
        # Resolve first sets.
        #
        self.calc_first_sets(self.epoch_symbol)
        #
        # Create the parser.
        #
        start_state, lr0_item_sets = self.compute_lr1_items(self.epoch_symbol)
        table = self.compute_parsing_table(lr0_item_sets)
        self.parsing_table = (start_state, table)
        self.normalise_item_set_id(self.parsing_table)
        self.disambiguate(table)

    #
    # Return the optimised_grammar suitable for passing to compile_grammar().
    #
    def pre_compile_grammar(self, grammar_class, pre_compiled=None):
        def from_flat(i, flat):
            return Lr1State.from_flat(i, flat, self.rules, self.token_registry)
        #
        # If it has already been pre compiled just return.
        #
        if self.parsing_table is not None:
            if isinstance(self.parsing_table[0], int):
                return None
        #
        # If we don't have a table see if we can use the pre_compiled version.
        #
        if pre_compiled:
            if isinstance(pre_compiled, string_types):
                pre_compiled = ast.literal_eval(pre_compiled)
            if pre_compiled[0] == self.grammar_hash():
                optimised_start_state = pre_compiled[1]
                optimised_parsing_table = tuple(
                    from_flat(i - 2, pre_compiled[i])
                    for i in range(2, len(pre_compiled)))
                self.parsing_table = (
                    optimised_start_state, optimised_parsing_table)
                return None
        #
        # Optimise it.
        #
        self.compile_grammar()
        self.parsing_table = self.optimise_parsing_table(self.parsing_table)
        flattened = tuple(
            state.to_flat(grammar_class)
            for state in self.parsing_table[1])
        return repr((self.grammar_hash(), self.parsing_table[0]) + flattened)

    #
    # Make the start state item_set.id==0 and the remainder following
    # sequentially.
    #
    def normalise_item_set_id(cls, parsing_table):
        start_state, table = parsing_table
        mapping = iter(zip(ItemSet.sorted(table), itertools.count(0)))
        first = next(mapping)
        for item_set, id in itertools.chain((first,), mapping):
            item_set.id = id
        start_state.id, first[0].id = first[0].id, start_state.id
    normalise_item_set_id = classmethod(normalise_item_set_id)

    #
    # Debug dump of nonterms.
    #
    def repr_productions(self):
        def r(nonterm):
            all_nonterms.add(nonterm)
            for symbol in nonterm.nested:
                if isinstance(symbol, Nonterm) and symbol not in all_nonterms:
                    r(symbol)
        all_nonterms = set()
        for rule in self.rules.values():
            r(rule)
        all_nonterms = [n for n in sorted(all_nonterms, key=lambda t: str(t))]
        result = []
        i = 0
        for nonterm in all_nonterms:
            if not nonterm.productions:
                continue
            rank = ".%d" % nonterm.rank if nonterm.rank else ""
            result.append(
                "%-6s: %r" % ("%d%s" % (i, rank), nonterm.productions[0],))
            for p in nonterm.productions[1:]:
                result.append("        %r" % (p,))
            i += 1
        return '\n'.join(result)

    #
    # Debug dump of the parsing table.
    #
    def repr_parse_table(self, state=None):
        if self.parsing_table is None:
            return ''
        a_state = next(iter(self.parsing_table[1]))
        if state is None:
            func = lambda item_set: True
        elif state >= 0:
            func = lambda item_set: item_set.id == state
        else:
            is_state = lambda s: (s if isinstance(s, int) else s.id) == -state
            action_state = lambda act: is_state(tuple(act)[0])
            func = lambda item_set: (
                is_state(item_set) or
                any(is_state(g) for g in item_set.gotos.values()) or
                any(action_state(a) for a in item_set.actions.values()))
        item_sets = (i for i in self.parsing_table[1] if func(i))
        result = []
        for item_set in a_state.sorted(item_sets):
            result.append(repr(item_set))
            result.append("")
        if result:
            del result[-1]
        return '\n'.join(result)

    #
    # Dump the grammar.
    #
    def repr_grammar(self):
        result = [repr(rule) for name, rule in sorted(self.rules.items())]
        return '\n'.join(result)

    #
    # Parse a feed.
    #
    def parse(self, input, tree_factory=None, on_error=None, log=None):
        if self.parsing_table is None:
            self.compile_grammar()
        token_feed = self.token_registry.tokeniser(input, self.whitespace)
        return self.lr1_parser(token_feed, tree_factory, on_error, log)

    #
    # Build up a catalogue of all symbols.
    #
    def catalogue_symbols(cls, dct):
        #
        # Move one symbols dict to another.
        #
        def move_dict(to, frm):
            for key in list(frm.dict):
                to.dict[key] = frm.dict[key]
                del frm.dict[key]

        #
        # Create a new Rule() for this rule.
        #
        def catalogue(name, field):
            rule = Rule(name, field)
            rule_symbols[field] = rule
            symbols[name] = rule
            dct[name] = rule
            return rule

        rule_symbols = {}
        symbols = {}
        token_registry = None
        for name, field in sorted(dct.items()):
            if isinstance(field, Symbol):
                if isinstance(field, Ref):
                    raise GrammarError("Ref(%r) hasn't been defined" % name)
                if field not in rule_symbols:
                    move_dict(catalogue(name, field), field)
                elif name == "START":
                    catalogue(name, rule_symbols[field])
                elif rule_symbols[field].name == "START":
                    rule = rule_symbols[field]
                    field_rule = catalogue(name, field)
                    move_dict(field_rule, rule)
                    catalogue(rule.name, field_rule)
                else:
                    msg = (
                        "You have \"%s = %s\" or " +
                        "\"%s = TokenRegistry.tok; " +
                        "%s = TokenRegistry.tok\".\n" +
                        "Only START maybe assigned directly to " +
                        "another Symbol. A workaround is %s = %s * 1"
                    )
                    raise GrammarError(msg % (
                        name, rule_symbols[field],
                        name, rule_symbols[field],
                        name, rule_symbols[field],))
            elif isinstance(field, type) and TokenRegistry in field.__bases__:
                if token_registry is None:
                    token_registry = field
                else:
                    msg = "Can't have more than one %s"
                    raise GrammarError(msg, TokenRegistry.__name__)
        return rule_symbols, symbols, token_registry
    catalogue_symbols = classmethod(catalogue_symbols)

    #
    # Replace all occurrences of a declared symbol with it's Rule().
    #
    def resolve_rule(self, rule, rule_symbols):
        def r(nested):
            for i, symbol in zip(itertools.count(), nested):
                if not isinstance(symbol, Rule):
                    resolved = rule_symbols.get(symbol, None)
                    if resolved is not None:
                        nested[i] = resolved
                    else:
                        r(symbol.nested)
        for i, symbol in zip(itertools.count(), rule.nested):
            # resolved = rule_symbols[symbol]
            # if resolved is not rule:
            #     rule.nested[i] = resolved
            assert rule_symbols[symbol] is rule
            if not isinstance(symbol, Rule):
                r(symbol.nested)

    #
    # Scan the entire grammar, allowing nodes in the parse tree to replace
    # themselves with other nodes. Eg, Ref's with the Symbol they are
    # referencing.
    #
    def resolve_symbol(self, rule):
        def r(parent, symbol):
            resolved = symbol.resolve_symbol(name, self.rules, token_registry)
            if isinstance(resolved, Rule):
                return resolved
            resolved.parent = parent
            for i, sym in zip(itertools.count(), resolved.nested):
                if not isinstance(sym, Rule):
                    resolved.nested[i] = r(resolved, sym)
            return resolved
        name = rule.name
        token_registry = self.token_registry
        for i, sym in zip(itertools.count(), rule.nested):
            rule.nested[i] = r(rule, sym)

    #
    # Compute the first sets for all symbols.
    #
    def calc_first_sets(self, epoch_symbol):
        #
        # Collect all nonterminals used by the grammar.
        #
        def r(nonterm):
            for prod in nonterm.productions:
                for sym in prod.rhs:
                    if isinstance(sym, Nonterm) and sym not in nonterms:
                        nonterms.add(sym)
                        nonterm_list.append(sym)
                        r(sym)
        #
        # We use nonterm_list to make it deterministic.
        #
        nonterms = set()
        nonterm_list = []
        r(epoch_symbol)
        changed = True
        while changed:
            changed = any(sym.merge_first_set(self) for sym in nonterm_list)

    #
    # Compute the collection of sets of LR(1) items.
    #
    # Wikipedia is a good reference:
    #   http://en.wikipedia.org/wiki/Canonical_LR_parser
    #
    def compute_lr1_items(self, epoch_symbol):
        #
        # Initialise the parse table by creating the start ItemSet.  It
        # contains one item:
        #
        #   FINISH ::= ^ START $., <e>
        #
        cache = {'__empty__': self.empty_token}
        start_production = epoch_symbol.productions[0]
        start_item_lr0 = Lr0Item(start_production, 0, cache)
        start_item_lookahead = set(self.empty_token.first_set)
        start_item_lr1 = Lr1Item(start_item_lr0, start_item_lookahead)
        start_item_set = ItemSet(((start_item_lr1, {(): set()}),), cache)
        start_item_set.compute_closure(cache)
        lr0_item_sets = collections.defaultdict(list)
        lr0_item_sets[start_item_set.lr0_kernel].append(start_item_set)
        #
        # Now compute new ItemSet's from the ones we have created, until we
        # all we create is duplicates.
        #
        worklist = collections.deque([start_item_set])
        while worklist:
            item_set = worklist.popleft()
            goto_sets = item_set.goto_sets(cache).items()
            for symbol, goto_set in sorted(goto_sets, key=lambda i: i[0].id):
                lr1_merge = None
                for lr1_item_set in lr0_item_sets[goto_set.lr0_kernel]:
                    if lr1_item_set.compatible(goto_set):
                        lr1_merge = lr1_item_set
                        break
                if lr1_merge is None:
                    goto_set.compute_closure(cache)
                    worklist.append(goto_set)
                    lr0_item_sets[goto_set.lr0_kernel].append(goto_set)
                elif lr1_merge.merge(goto_set, cache):
                    if lr1_merge in worklist:
                        worklist.remove(lr1_merge)
                    worklist.appendleft(lr1_merge)
        return start_item_set, lr0_item_sets

    #
    # Compute LR(1) actions.
    #
    def compute_parsing_table(cls, lr0_item_sets):
        #
        # First assign a unique ID to each item_set.  This ID will be it's
        # index into actions[] and gotos[].
        #
        table = dict(
            (item_set, item_set)
            for item_set_list in lr0_item_sets.values()
            for item_set in item_set_list)
        for item_set in table:
            #
            # Compute actions.
            #
            actions = collections.defaultdict(set)
            goto_sets = item_set.goto_sets(None)
            for item in item_set.all_items():
                dot_pos, rhs = item.dot_pos, item.production.rhs
                if dot_pos == len(rhs):
                    for token in item.lookahead:
                        actions[token].add(ReduceAction(item))
                elif isinstance(rhs[item.dot_pos], TokenSymbol):
                    token = rhs[item.dot_pos]
                    goto_set = goto_sets[token]
                    found = False
                    for lr1_item_set in lr0_item_sets[goto_set.lr0_kernel]:
                        if lr1_item_set.compatible(goto_set):
                            actions[token].add(ShiftAction(lr1_item_set))
                            found = True
                            break
                    assert found, repr(token)
            #
            # Turn the action lists into tuples.
            #
            item_set.actions = dict(actions)
            for token in actions:
                item_set.actions[token] = tuple(item_set.actions[token])
            #
            # Compute goto's.
            #
            gotos = item_set.gotos
            for symbol in goto_sets:
                if not isinstance(symbol, Nonterm):
                    continue
                goto_set = goto_sets[symbol]
                for lr1_item_set in lr0_item_sets[goto_set.lr0_kernel]:
                    if lr1_item_set.compatible(goto_set):
                        assert symbol not in gotos
                        gotos[symbol] = lr1_item_set
                        break
        return table
    compute_parsing_table = classmethod(compute_parsing_table)

    #
    # Look for action ambiguities and resolve them if possible.
    #
    def disambiguate(cls, item_sets):
        action_list = [
            (item_set, act)
            for item_set in item_sets
            for act in item_set.actions.items()]
        for item_set, (token, actions) in action_list:
            if len(actions) == 1:
                item_set.actions[token] = actions[0]
                continue
            #
            # Multiple actions are ambiguities.  Compare every action with
            # all others in the hope that Prio() and Assoc() can eliminate
            # all bar one.
            #
            new_actions = sorted(actions)       # Repeatability for testing
            i = 0
            while i < len(new_actions) - 1:
                act_0 = new_actions[i]
                j = i + 1
                while j < len(new_actions):
                    act_1 = new_actions[j]
                    keep = cls.resolve_ambiguity(item_set, token, act_0, act_1)
                    if "1" not in keep:
                        del new_actions[j]
                        j -= 1
                    j += 1
                    if "0" not in keep:
                        del new_actions[i]
                        i -= 1
                        break
                i += 1
            #
            # Since we don't support GLR(1) grammars yet (ie, we don't do
            # Split()), resolve_ambiguity() must have not more than one result
            # left.  It's possible Nonassoc eliminates all of them.
            #
            if not new_actions:
                del item_set.actions[token]
            else:
                assert len(new_actions) == 1
                item_set.actions[token] = new_actions[0]
    disambiguate = classmethod(disambiguate)

    #
    # Compute how to resolve an action conflict.  Returns the actions to
    # keep, or "err".
    #
    def resolve_ambiguity(cls, item_set, token, action_0, action_1):
        #
        # Print a nice and hopefully useful error message when we can't
        # resolve a conflict.
        #
        def err(reason):
            #
            # Print an action in a nice looking way.
            #
            def explain_action(action):
                if isinstance(action, ReduceAction):
                    rhs = ' '.join(str(s) for s in action[3].production.rhs)
                    lhs = action[3].production.lhs
                    return ["  replace the sequence [%s] with %s" % (rhs, lhs)]
                next_state_lr1_items = list(action[0])
                one_of = "" if len(next_state_lr1_items) == 1 else " one of"
                msg = [
                    "  accept the %s in the hope it will match%s:"
                    % (token, one_of)
                ]
                msg.extend([
                    "    " + repr(lr1_item.lr0_item)
                    for lr1_item in Lr1Item.sorted(next_state_lr1_items)])
                return msg

            #
            # Is the passed item relevant to the action?
            #
            def relevant(item, action):
                if isinstance(action, ReduceAction):
                    return action[3] is item
                return (item.dot_pos < len(item.production.rhs) and
                        item.production.rhs[item.dot_pos] == token)
            #
            # Produce a nicely formatted error message.
            #
            msg = ["Conflict: %s" % (reason,)]
            msg.append("While trying to recognise state %d:" % (item_set.id,))
            for item in item_set.all_items():
                if relevant(item, action_0) or relevant(item, action_1):
                    msg.append("  %r" % (item.lr0_item,))
            msg.append("on seeing a %s I could not decide between:" % (token,))
            msg.extend(explain_action(action_0))
            msg.append("and")
            msg.extend(explain_action(action_1))
            msg.append("Please remove this ambiguity from your grammar")
            raise GrammarError('\n'.join(msg))

        #
        # Decide the associativity of a symbol.
        #
        def assoc(nonterm):
            if isinstance(nonterm, Assoc):
                return nonterm
            return None
        #
        # Ambiguities can arise from productions like this:
        #
        #   e = e op e
        #   op = op1 | op2
        #
        # When confronted with the token string:
        #
        #   e op1 e ^ op2 e
        #
        # We get a shift / reduce conflict at the indicated position.
        # The choice is really between two different parse trees:
        #
        #   Shift:   (e op1 (e op2 e))
        #   Reduce:  ((e op1 e) op2 e)
        #
        # This can be resolved in two ways.  If priorities are allocated to
        # the clashing productions, then we can choose the action the
        # grammar writer preferred based on which of 'op1' or 'op2' was
        # used:
        #
        #   e = Prio(e op1 e, e op2 e)
        # which is equivalent to:
        #   e = e op1 e, Priority=0
        #   e = e op2 e, Priority=1
        #
        # Or, the grammar writer can specify associativity, ie saying he always
        # wants ((e <op> 2) <op> e) regardless of 'op' (left associative) or he
        # wants (e op (e op e)) regardless of 'op' (right associative).  For
        # left associativity it is like this:
        #
        #   e = e << op << e
        #
        # A reduce/reduce conflict can happen if the clash happens higher up
        # in the parse tree:
        #
        #   e0 = Prio(e1, e2)
        #   e1 = 'n'
        #   e2 = 'n'
        #
        # When parsing:
        #
        #   'n' ^
        #
        # We have two reduction choices, e1='n' and e2='n'.  These productions
        # don't have priorities allocated directly, but the parse table builder
        # will have pushed the priorities e0 allocated down to them.  Nested
        # priorities are ranked (see allocate_rank()), and this ranking makes
        # priorities globally comparable.
        #
        lhs_0, low_0, high_0 = action_0.precedence(token, item_set)
        lhs_1, low_1, high_1 = action_1.precedence(token, item_set)
        if low_0 < low_1:
            return "0"
        if low_0 > low_1:
            return "1"
        #
        # The priorities are the same.  Try using the associativity.
        #
        assert (
            not isinstance(action_0, ShiftAction) or
            not isinstance(action_1, ShiftAction))
        if (isinstance(action_0, ReduceAction) and
                isinstance(action_1, ReduceAction)):
            return err("Reduce/Reduce")
        lhs_0_assoc, lhs_1_assoc = assoc(lhs_0), assoc(lhs_1)
        lhs = lhs_0_assoc if lhs_0_assoc is not None else lhs_1_assoc
        if not isinstance(lhs, Assoc):
            return err("Shift/Reduce and no associativity")
        if isinstance(lhs_1_assoc, Assoc) and lhs.assoc != lhs_1_assoc.assoc:
            return err("Shift/Reduce and conflicting associativity")
        #
        # The actions have met the associativity pre-conditions so we can
        # resolve the conflict.
        #
        if lhs.assoc == 'l':    # Left()
            return "1" if isinstance(action_0, ShiftAction) else "0"
        if lhs.assoc == 'r':    # Right()
            return "0" if isinstance(action_0, ShiftAction) else "1"
        assert lhs.assoc == 'n', lhs_0.assoc
        return ""               # Nonassoc() - Association is a parse error
    resolve_ambiguity = classmethod(resolve_ambiguity)

    #
    # This optional step creates a parsing_table with all the information
    # in the ItemSet() pruned.
    #
    def optimise_parsing_table(self, parsing_table):
        start_state, table = parsing_table
        #
        # Create a mapping of item_set: int, with start_state mapping to 0.
        #
        state_number = dict(zip(ItemSet.sorted(table), itertools.count()))
        item_set_0 = next(i for i in state_number if state_number[i] == 0)
        state_number[item_set_0] = state_number[start_state]
        state_number[start_state] = 0
        #
        # Map each item set to it's number.
        #
        optimised = []
        for item_set in sorted(table, key=lambda i: state_number[i]):
            actions = {}
            for token in item_set.actions:
                action = item_set.actions[token]
                if isinstance(action, ShiftAction):
                    new_action = (state_number[action[0]],)
                else:
                    nonterm_nr = action[0].id
                    len_rhs = action[1]
                    output = action[2]
                    new_action = (nonterm_nr, len_rhs, output, None)
                actions[token] = new_action
            gotos = dict(
                (nonterm.id, state_number[itm_set])
                for nonterm, itm_set in item_set.gotos.items())
            state_id = len(optimised)
            lr1_state = Lr1State(state_id, actions, gotos, item_set.rules)
            optimised.append(lr1_state)
        #
        # And we are done.
        #
        return 0, optimised

    #
    # The grammar hash.
    #
    def grammar_hash(self):
        grammar_tree = '; '.join(sorted(
            repr(self.rules[name]) for name in self.rules))
        hsh = hashlib.sha512()
        hsh.update(grammar_tree.encode())
        hsh.update(self.VERSION.encode())
        return hsh.hexdigest()

    #
    # The LR(1) table driven parser.
    #
    def lr1_parser(self, token_feed, tree_factory, on_error, log):
        #
        # Print the current stack, for log.
        #
        def print_stack(stk):
            return " ".join(
                "%s=%s" % (s[0], s[1][0][0] if s[1] else '()') for s in stk)

        #
        # The lengths I am prepared to go in the name of fast path
        # efficiency frightens me a times.
        #
        def insert_error_recovery_tuple():
            while recovery_stack:
                input_tuple = next(recovery_stack[-1], EOF)
                if input_tuple is not EOF:
                    yield input_tuple
                else:
                    recovery_stack.pop()
            iterator[0] = original_input
            yield next(iterator[0], EOF)

        EOF = object()
        recovery_stack = []
        original_input = itertools.chain(token_feed, ((self.eoi_token,),))
        iterator = [original_input]
        start_state, table = self.parsing_table
        state = table[start_state]
        stack = [(state, ((self.empty_token,),))]
        if not self.comment_tokens:
            comments = ()
        else:
            comments = frozenset(self.comment_tokens)
        input_tuple = next(iterator[0], EOF)
        while input_tuple is not EOF:
            token = input_tuple[0]
            if token in comments:
                input_tuple = next(iterator[0], EOF)
                continue
            while True:
                try:
                    action = state.actions[token]
                except KeyError:
                    #
                    # A Parse error.  Does he want to do error recovery?
                    #
                    if on_error is None:
                        raise ParseError(input_tuple, stack)
                    insert = on_error(iterator[0], input_tuple, stack)
                    if insert is None:
                        raise ParseError(input_tuple, stack)
                    recovery_stack.append(iter(insert))
                    iterator = [insert_error_recovery_tuple()]
                    break
                #
                # A shift?
                #
                if len(action) == 1:
                    if token is self.eoi_token:
                        break
                    if tree_factory:
                        input_tuple = tree_factory(input_tuple)
                    state = table[action[0]]
                    stack.append((state, (input_tuple,)))
                    if log:
                        log("shift  %s; %s" % (token, print_stack(stack[1:]),))
                    break
                #
                # A reduce.
                #
                goto, pop_count, output, _ = action
                if pop_count == 0:
                    tail = ()
                    nodes = ()
                else:
                    tail = stack[-pop_count:]
                    del stack[-pop_count:]
                    nodes = sum((s[1] for s in tail), ())
                if output is not None:
                    nodes = (output,) + nodes
                    if tree_factory:
                        nodes = tree_factory(nodes)
                    nodes = (nodes,)
                state = table[stack[-1][0].gotos[goto]]
                stack.append((state, nodes))
                if log:
                    log(
                        "reduce %s; %s -- %s" %
                        (token, print_stack(tail), print_stack(stack[1:])))
            input_tuple = next(iterator[0], EOF)
        return stack[-1][1][0]


#
# The base class for Symbol's in the grammar: a token or a non-terminal.
#
class Symbol(object):
    DICT_METHODS = (
        "__contains__", "__delitem__", "__getitem__", "__iter__",
        "__len__", "__setitem__")
    __slots__ = ('dict', 'id', 'first_set', 'nested', 'parent') + DICT_METHODS
    SYMBOL_PRECEDENCE = 0

    def __init__(self):
        self.dict = {}
        self.first_set = frozenset()
        self.nested = ()
        self.parent = None
        #
        # The original idea was to inherit from dict.  Doing so meant Symbol
        # wasn't hashable and since it is used extensively in sets and as
        # keys to dict's overriding __hash__ so it was hash'able caused a
        # 10% slowdown in generating the parse table.  So now we just emulate
        # a dict.
        #
        self.__contains__ = self.dict.__contains__
        self.__delitem__ = self.dict.__delitem__
        self.__getitem__ = self.dict.__getitem__
        self.__iter__ = self.dict.__iter__
        self.__len__ = self.dict.__len__
        self.__setitem__ = self.dict.__setitem__

    def __add__(self, other):
        return OpPlus(self, other)

    def __radd__(self, other):
        return OpPlus(other, self)

    def __mul__(self, other):
        if other is Opt or other is Some or other is Many or other is Repeat:
            return other(self)
        if isinstance(other, int):
            return Repeat(self, other, other)
        if (not isinstance(other, tuple) or len(other) > 2 or
                any(not isinstance(a, int) for a in other)):
            msg = (
                "right operand of * must be one of: " +
                "Opt, Some, Many, Repeat, (), (min,), (min,max)")
            raise GrammarError(msg)
        return Repeat(self, *other)

    def __rmul__(self, other):
        if other is Opt or other is Some or other is Many or other is Repeat:
            return other(self)
        if isinstance(other, int):
            return Repeat(self, other, other)
        if (not isinstance(other, tuple) or len(other) > 2 or
                any(not isinstance(a, int) for a in other)):
            msg = (
                "left operand of * must be one of: " +
                "Opt, Some, Many, Repeat, (), (min,), (min,max)")
            raise GrammarError(msg)
        return Repeat(self, *other)

    def __lshift__(self, other):
        return OpLshift(self, other)

    def __rlshift__(self, other):
        return OpLshift(other, self)

    def __rshift__(self, other):
        return OpRshift(self, other)

    def __rrshift__(self, other):
        return OpRshift(other, self)

    def __or__(self, other):
        return OpOr(self, other)

    def __ror__(self, other):
        return OpOr(other, self)

    def __nonzero__(self):
        return True
    __bool__ = __nonzero__                      # For Python3

    def cast(cls, value):
        value = cls.CAST.get(type(value), lambda x: x)(value)
        if not isinstance(value, Symbol):
            raise GrammarError("%r can't be a Symbol" % value)
        return value

    def compile_symbol(self, comp):
        raise NotImplementedError()

    def resolve_symbol(self, name, rules, token_registry):
        return self

    #
    # Return a unique name for the symbol.
    #
    def __str__(self):
        if self.parent is None:
            return self.__class__.__name__
        same_as_me = tuple(
            sym
            for sym in self.parent.nested
            if sym.__class__.__name__ == self.__class__.__name__)
        if len(same_as_me) < 2:
            my_name = self.__class__.__name__
        else:
            index = same_as_me.index(self)
            my_name = "%s%d" % (self.__class__.__name__, index)
        return "%s.%s" % (self.parent, my_name,)

    def __repr__(self):
        return "%s(%s)" % (self.__class__.__name__, self.repr_nested())

    def repr_nested(self):
        return ', '.join(sym.nested_repr() for sym in self.nested)

    def nested_repr(self):
        return repr(self)

    def get_rule(self):
        rule = self
        while rule.parent is not None:
            rule = rule.parent
        assert isinstance(rule, Rule)
        return rule


#
# A forward reference to a symbol that will be defined later.
#
class Ref(Symbol):
    __slots__ = ('referenced_name',)

    def __init__(self, referenced_name):
        super(Ref, self).__init__()
        self.referenced_name = referenced_name

    def resolve_symbol(self, name, rules, token_registry):
        if self.referenced_name not in rules:
            raise GrammarError("%s references undefined %r" % (name, self))
        if len(self.dict) != 0:
            msg = "Ref(%s) may not have dictionary elements"
            raise GrammarError(msg % (self.referenced_name,))
        return rules[self.referenced_name]

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self.referenced_name)


#
# A reference to the Rule() of the current production.
#
class This(Symbol):
    __slots__ = ()

    def resolve_symbol(self, name, rules, token_registry):
        return rules[name]


THIS = This()


#
# Combine several lists productions as a sequence.  In other words, say we
# had several nonterms which were combined as a sequence:
#
#    a = a0 + a1 + a2
#
# a0, a1 and a2 have their own lists of productions.  We must produce the
# productions for a from those lists.  So:
#
#   Given:
#     a0 = [(a,),(b,)]
#     a1 = [(c,d),(e,f)]
#     a2 = [(g,)]
#
#   Return:
#     [(a,c,d,g), (a,e,f,g), (b,c,d,g), (b,e,f,g)]
#
def seq(*args):
    if not isinstance(args[-1], int):
        repeats = 1
    else:
        repeats = args[-1]
        args = args[:-1]
    args = [[(p,)] if isinstance(p, Symbol) else p for p in args]
    result = list(
        tuple(itertools.chain.from_iterable(prod))
        for prod in itertools.product(*args, repeat=repeats))
    return result


#
# A non-terminal: A symbol that appears on the left hand side of a rule.
#
class Nonterm(Symbol):
    __slots__ = ('rank', 'productions', 'priority')

    def __init__(self, *args):
        super(Nonterm, self).__init__()
        for symbol in args:
            if isinstance(symbol, Rule) and symbol.resolved:
                msg = "Can not import %s from another Grammar" % (
                    symbol.name,)
                raise GrammarError(msg)
            if isinstance(symbol, MetaToken):
                msg = "%s can not be used in a production" % (symbol.name,)
                raise GrammarError(msg)
        self.nested = [self.cast(symbol) for symbol in args]
        self.priority = None
        self.productions = ()
        self.rank = None

    #
    # Compile the grammar.  In other words turn it into a series of LR
    # productions.  Start from the start symbol and work our way down.
    # Record what symbols were referenced, as unreferenced ones are
    # probably an error.
    #
    def compile_grammar(self, empty_token):
        #
        # A recursive function that visits all symbols in the grammar,
        # compiling them.
        #
        def comp(symbol):
            if not isinstance(symbol, Rule):
                return symbol.compile_symbol(comp)
            if symbol not in seen:
                seen.add(symbol)
                symbol.emit(symbol.compile_symbol(comp))
            return [(symbol,)]
        seen = set()
        comp(self)
        self.allocate_symbol_id(empty_token)
        self.allocate_rank()
        return seen

    #
    # Allocate Nonterm.rank, so priorities can be calculated during the
    # parser generator process.  Lets say we have:
    #
    #    START = Prio(a, b)
    #    a     = Prio(b, c)
    #    b     = 'n'
    #    c     = Prio(START, a)
    #
    # So START has allocated 'b' a priority of 1, but 'a' has allocated 'b' a
    # priority of 0.  The question is how do we compare these?  The answer
    # used here is to rank the priorities based on how close they are to
    # the START symbol.  START is 0 away from itself, so it has the most
    # preferred rank.  'a' and 'b' have ranks of 1, because they are referred
    # to directly by START, and 'c' has a rank of 2.
    #
    # The parser uses this rank to create a priority tuple: (p0, p1, p2, ...),
    # where p0 is the assigned by rank 0, p1 is the priority assigned by rank 1
    # and so on.
    #
    # So in answering the question earlier, START's priority for 'b' is:
    #   (START = b,) which is (1,),
    # and 'a's priority for 'b' is:
    #   (START = a, a = b,) which is (0,0,).
    #
    # When parsing this sequence of symbols:
    #
    #    'n' __end_of_input__
    #
    # The parser will go through these states.
    #
    #   STACK   INPUT            POSSIBLE ACTIONS
    #   []      'n'              shift 'n'
    #   ['n']   __end_of_input__ reduce b = 'n'
    #   [b]     __end_of_input__ reduce a = b, reduce START = b
    #
    # The priority of a = b is (0,0), and the priority of START = b is (1,).
    # Since (0,0) < (1,), the parser will chose a = b.
    #
    def allocate_rank(self):
        #
        # First determine the height of each Nonterm in the production tree
        # by doing a a breadth first pass over it.  Collect the tree structure
        # as we go.
        #
        height = 0
        refers = collections.defaultdict(set)
        refers[self].add(self)
        symbol_heights = {self: height}
        height += 1
        queue = [self]
        while queue:
            queue, process = [], queue
            for nonterm in process:
                refers[nonterm] |= frozenset()
                for production in nonterm.productions:
                    for symbol in production.rhs:
                        if isinstance(symbol, Nonterm):
                            if nonterm is not symbol:
                                refers[symbol].add(nonterm)
                            if symbol not in symbol_heights:
                                symbol_heights[symbol] = height
                                queue.append(symbol)
            height += 1

        #
        # Turn it into a hierarchy.  The idea is we do a topological sort on
        # the graph using the tree structure we accumulated, breaking any
        # cycles using height.
        #
        def remove(no_refers):
            no_prio = set(
                s for s in no_refers
                if not isinstance(s, Prio.Prioritised))
            if not no_prio:
                rank[0] += 1
            else:
                no_refers = no_prio
            for s in no_refers:
                s.rank = rank[0]
                del refers[s]
            for s in refers:
                refers[s] -= no_refers
        rank = [0]
        while refers:
            no_refers = set(s for s, r in refers.items() if not r)
            while refers and no_refers:
                remove(no_refers)
                no_refers = set(s for s, r in refers.items() if not r)
            #
            # If that didn't consume every token we have a cycle.  Break the
            # cycle by considering the node closest to the START symbol (ie
            # lowest height) as havingt the heighest priority.
            #
            if refers:
                low = min(symbol_heights[s] for s in refers)
                lowest = set(s for s in refers if symbol_heights[s] == low)
                remove(lowest)
        # print sorted("%d %s" % (sym.rank, sym) for sym in symbol_heights)

    #
    # Allocate each symbol a unique id.  This is used to make things
    # deterministic when we iterate through hash tables and sets.
    #
    def allocate_symbol_id(self, empty_token):
        queue = collections.deque([self, empty_token])
        all_symbols = set()
        while queue:
            symbol = queue.popleft()
            if symbol in all_symbols:
                continue
            symbol.id = len(all_symbols)
            all_symbols.add(symbol)
            if isinstance(symbol, Nonterm):
                for production in symbol.productions:
                    queue.extend(production.rhs)

    #
    # Generate the Production() objects for the Nonterm().
    #
    def emit(self, productions):
        self.productions = [Production(self, p) for p in productions]

    #
    # Merge the first_sets of all productions into ours.  Return True if
    # the first_set was changed.
    #
    def merge_first_set(self, parser):
        want_empty = False
        have_empty = parser.empty_token in self.first_set
        result = self.first_set
        for production in self.productions:
            #
            # For all A = a + b, merge first(a) into first(A).
            #
            for symbol in production.rhs:
                result |= symbol.first_set
                if parser.empty_token not in symbol.first_set:
                    break
                have_empty = True
            else:
                want_empty = True
        if want_empty:
            if not have_empty:
                result |= parser.empty_token.first_set
        else:
            if have_empty:
                result -= parser.empty_token.first_set
        if len(result) != len(self.first_set):
            self.first_set = result
            return True
        return False

    #
    # For nonterms that accept lists, print in a nice way.
    #
    def nonterm_repr(self, delimiter):
        def repr_sym(symbol):
            if (len(symbol.nested) < 2 or
                    self.SYMBOL_PRECEDENCE <= symbol.SYMBOL_PRECEDENCE):
                return symbol.nested_repr()
            return '(' + symbol.nested_repr() + ')'
        if len(self.nested) < 2:
            return "%s(%s)" % (
                self.__class__.__name__,
                ', '.join(sym.nested_repr() for sym in self.nested),)
        nested = (repr_sym(symbol) for symbol in self.nested)
        return (' ' + delimiter + ' ').join(nested)


#
# A symbol on the left hand side of a grammar rule.
#
class Rule(Nonterm):
    __slots__ = ('name', 'resolved')

    def __init__(self, name, symbol):
        super(Rule, self).__init__(symbol)
        self.name = name
        self.resolved = False

    def compile_symbol(self, comp):
        return comp(*self.nested)

    def __str__(self):
        return self.name

    def __repr__(self):
        if isinstance(self.nested[0], Rule):
            return "%s = %s" % (self.name, self.nested[0].name)
        return "%s = %r" % (self.name, self.nested[0])

    def nested_repr(self):
        return "%s" % (self.name,)


#
# The arguments are a single production.  Thus:
#
#    L = Sequence(sym0, sym1, sym2)
#
# Yields the production:
#
#    L ::= sym0 sym1 sym2
#
class Sequence(Nonterm):
    __slots__ = ()
    SYMBOL_PRECEDENCE = 3

    def __init__(self, *args):
        super(Sequence, self).__init__(*args)

    def compile_symbol(self, comp):
        return seq(*[comp(symbol) for symbol in self.nested])

    def __repr__(self):
        return self.nonterm_repr('+')


#
# The arguments are alternate productions. Thus
#
#    L = Choice(sym0, sym1, sym2)
#
# Yields the productions:
#
#    L ::= sym0
#    L ::= sym1
#    L ::= sym2
#
class Choice(Nonterm):
    __slots__ = ()
    SYMBOL_PRECEDENCE = 1

    def __init__(self, *args):
        super(Choice, self).__init__(*args)

    def compile_symbol(self, comp):
        return sum([comp(sym) for sym in self.nested], [])

    def __repr__(self):
        return self.nonterm_repr('|')


#
# Binary operations.  Repeated applications of the same binary operation
# yield a single list.  The op is then applied to that list.  Thus:
#
#   L = sym0 + sym1 + sym2 | sym3 | sym4
#
# Gets turned into:
#
#   L = Alternate(Sequence(sym0, sym1, sym2), sym3, sym4).
#
class BinOp(Nonterm):
    __slots__ = ()

    def __init__(self, arg1, arg2):
        super(BinOp, self).__init__(arg1, arg2)

    def combine(self):
        def r(arg1, arg2):
            a1 = r(*arg1.nested) if type(self) == type(arg1) else [arg1]
            a2 = r(*arg2.nested) if type(self) == type(arg2) else [arg2]
            return a1 + a2
        return r(*self.nested)


#
# Constructed for: sym | sym.
#
class OpOr(BinOp):
    __slots__ = ()

    def resolve_symbol(self, name, rules, token_registry):
        return Choice(*self.combine())


#
# Constructed for: sym + sym.
#
class OpPlus(BinOp):
    __slots__ = ()

    def resolve_symbol(self, name, rules, token_registry):
        return Sequence(*self.combine())


#
# Constructed for: sym << sym.
#
class OpLshift(BinOp):
    __slots__ = ()

    def resolve_symbol(self, name, rules, token_registry):
        return Left(*self.combine())


#
# Constructed for: sym >> sym.
#
class OpRshift(BinOp):
    __slots__ = ()

    def resolve_symbol(self, name, rules, token_registry):
        return Right(*self.combine())


#
# A priority list.  A priority list is passed a list of symbols:
#    S = Prio(sym0, sym1, sym2)
# is identical to:
#    S = sym0 | sym1 | sym2
# with the side effect that in the event of a conflict we will choose
# sym0 over sym1 over sym2.
#
class Prio(Nonterm):
    #
    # This node holds priorities.
    #
    class Prioritised(Nonterm):
        __slots__ = ()

        def __init__(self, priority, symbol):
            super(Prio.Prioritised, self).__init__(symbol)
            self.priority = priority

        def compile_symbol(self, comp):
            self.emit(comp(*self.nested))
            return [(self,)]

    __slots__ = ()

    def __init__(self, *args):
        super(Prio, self).__init__(*args)

    #
    # Priority nodes allocate a separate node for each sub-node.  We
    # subsume nested Prio's.
    #
    def resolve_symbol(self, name, rules, token_registry):
        def r(prio):
            for symbol in prio.nested:
                if isinstance(symbol, Prio):
                    r(symbol)
                else:
                    children.append(self.Prioritised(next(index), symbol))
        index = itertools.count()
        children = []
        r(self)
        self.nested = children
        return self

    def compile_symbol(self, comp):
        self.emit(sum([comp(sym) for sym in self.nested], []))
        return [(self,)]

    def __repr__(self):
        if len(self.nested) < 2:
            return "%s(%s)" % (
                self.__class__.__name__,
                ', '.join(sym.nested_repr() for sym in self.nested))
        return "(%s)" % (', '.join(sym.nested_repr() for sym in self.nested),)


#
# This node holds associativity: ie left, right or not allowed.
#
class Assoc(Nonterm):
    __slots__ = ('assoc',)

    def __init__(self, assoc, *args):
        if assoc not in 'lnr':
            raise GrammarError("Unknown associativity %r" % (assoc,))
        super(Assoc, self).__init__(*args)
        self.assoc = assoc

    def compile_symbol(self, comp):
        self.emit(seq(*[comp(symbol) for symbol in self.nested]))
        return [(self,)]

    def __repr__(self):
        if type(self) is not Assoc:
            return super(Assoc, self).__repr__()
        return "%s(%s, %s)" % (
            self.__class__.__name__, self.assoc, self.repr_nested(),)


#
# Force left associativity.
#
class Left(Assoc):
    __slots__ = ()
    SYMBOL_PRECEDENCE = 2

    def __init__(self, *args):
        super(Left, self).__init__('l', *args)

    def __repr__(self):
        return self.nonterm_repr('<<')


#
# Force right associativity.
#
class Right(Assoc):
    __slots__ = ()
    SYMBOL_PRECEDENCE = 2

    def __init__(self, *args):
        super(Right, self).__init__('r', *args)

    def __repr__(self):
        return self.nonterm_repr('>>')


#
# Force non associative.
#
class Nonassoc(Assoc):
    __slots__ = ()

    def __init__(self, *args):
        super(Nonassoc, self).__init__('n', *args)


#
# Construct a list of productions separated by a delimiter.
#
class List(Nonterm):
    __slots__ = ('max', 'min', 'opt',)

    def __init__(self, symbol, delimiter, min=None, max=None, opt=None):
        super(List, self).__init__(symbol, delimiter)
        self.min = 0 if min is None else min
        self.max = max
        self.opt = opt
        if max is not None and max < self.min:
            raise GrammarError("min may not be greater than max")

    def compile_symbol(self, comp):
        symbol = comp(self.nested[0])
        delimiter = comp(self.nested[1])
        productions = []
        if self.min == 0:
            productions.append(())
        #
        # A fixed max means we can just list all the possibilities like this:
        #
        #   [(), (sym,), (sym,delim,sym), (sym,delim,sym,delim,sym)]
        #
        if self.max is not None:
            for repeat in range(max(0, self.min - 1), self.max):
                prod = seq(symbol, seq(delimiter, symbol, repeat))
                productions.extend(prod)
                if self.opt:
                    productions.extend(seq(prod, delimiter))
            return productions
        #
        # There is no upper maximum, so we need recursion:
        #
        #  For right assoc:
        #    [(sym,delim,sym,delim,S)]
        #    S ::= [(sym,), (sym,delim,S)]
        #
        #  For left assoc:
        #    [(S,delim,sym,delim,sym)]
        #    S ::= [(sym,), (S,delim,sym)]
        #
        my_productions = list(symbol)
        if self.parent is None or not isinstance(self.parent, Assoc):
            assoc = None
        else:
            assoc = self.parent.assoc
        repeat = max(0, self.min - 1)
        if assoc == 'r':
            prod = seq(seq(symbol, delimiter, repeat), self)
            productions.extend(prod)
            my_productions.extend(seq(symbol, delimiter, self))
            if self.opt:
                my_productions.extend(seq(symbol, delimiter))
        elif assoc is None or assoc == 'l':
            prod = seq(self, seq(delimiter, symbol, repeat))
            productions.extend(prod)
            if self.opt:
                productions.extend(seq(prod, delimiter))
            my_productions.extend(seq(self, delimiter, symbol))
        else:
            msg = "Can't implement %s associativity on %r"
            raise GrammarError(msg % (assoc, self))
        self.emit(my_productions)
        return productions

    def __repr__(self):
        if self.opt is not None:
            return "%s(%s, %r, %r, %r)" % (
                self.__class__.__name__, self.repr_nested(),
                self.min, self.max, self.opt)
        if self.max is not None:
            return "%s(%s, %r, %r)" % (
                self.__class__.__name__, self.repr_nested(),
                self.min, self.max)
        if self.min != 0:
            return "%s(%s, %r)" % (
                self.__class__.__name__, self.repr_nested(),
                self.min)
        return "%s(%s)" % (self.__class__.__name__, self.repr_nested())


#
# Repeats.  This class handles all forms of repeats.
#
class Repeat(Nonterm):
    __slots__ = ('min', 'max',)

    def __init__(self, symbol, min=None, max=None):
        super(Repeat, self).__init__(symbol)
        self.min = 0 if min is None else min
        self.max = max
        if max is not None and max < self.min:
            raise GrammarError("min may not be greater than max")

    def compile_symbol(self, comp):
        symbol = comp(*self.nested)
        #
        # If we have both min and max repeats we can just enumerate the
        # results like this:
        #
        #   [ (), (symbol,), (symbol,symbol,), ... ]
        #
        if self.max is not None:
            prods = (seq(symbol, rpt) for rpt in range(self.min, self.max + 1))
            return sum(prods, [])
        #
        # There is no maximum, so we must use recursion.
        #
        #   For right associative:
        #     [ (symbol,symbol,S) ]
        #     S ::= [symbol, (symbol, S) ]
        #
        #   For left associative:
        #     [ (S,symbol,symbol) ]
        #     S ::= [symbol, (S,symbol) ]
        #
        if self.parent is None or not isinstance(self.parent, Assoc):
            assoc = None
        else:
            assoc = self.parent.assoc
        if self.min == 0:
            productions = [(), (self,)]
        else:
            productions = seq(seq(symbol, max(0, self.min - 1)), self)
        if assoc == 'r':
            my_productions = symbol + seq(symbol, self)
        elif assoc is None or assoc == 'l':
            my_productions = symbol + seq(self, symbol)
        else:
            msg = "Can't implement %s associativity on %r"
            raise GrammarError(msg % (assoc, self))
        self.emit(my_productions)
        return productions

    def __repr__(self):
        nested = self.nested[0].nested_repr()
        if len(self.nested[0].nested) >= 2:
            nested = '(%s)' % (nested,)
        if type(self) is not Repeat:
            return "%s * %s" % (nested, self.__class__.__name__)
        if self.min == self.max:
            return "%s * %r" % (nested, self.min)
        if self.max is not None:
            return "%s * (%r, %r)" % (nested, self.min, self.max)
        if self.min != 0:
            return "%s * (%r,)" % (nested, self.min)
        return "%s * ()" % (nested,)


#
# Optional - ie 0 or 1.
#
class Opt(Repeat):
    __slots__ = ()

    def __init__(self, symbol):
        super(Opt, self).__init__(symbol, 0, 1)


#
# 1 or more.
#
class Some(Repeat):
    __slots__ = ()

    def __init__(self, symbol):
        super(Some, self).__init__(symbol, 1, None)


#
# 0 or more.
#
class Many(Repeat):
    __slots__ = ()

    def __init__(self, symbol):
        super(Many, self).__init__(symbol, 0, None)


#
# Generate tokens by splitting a string.
#
class Tokens(Nonterm):
    __slots__ = ()

    def __init__(self, literals, keywords=None, case=None):
        tokens = []
        if literals and literals.strip():
            tokens.extend([
                Token(literal, case=case)
                for literal in literals.strip().split()])
        if keywords and keywords.strip():
            tokens.extend([
                Keyword(keyword, case)
                for keyword in keywords.strip().split()])
        super(Tokens, self).__init__(*tokens)

    def resolve_symbol(self, name, rules, token_registry):
        return Choice(*self.nested)


#
# The Tokeniser() breaks up input into tokens.
#
class Tokeniser(object):
    literals = None      # dict,    {"literal": token, ...}
    regex = None         # object,  re.compile() - compiled token recognisers
    re_groups = None     # tuple,   (int, ...)
    re_list = None       # tuple,   (Token(), ...)
    registry = None      # object,  TokenRegistry() that owns us
    unrecognised = None  # object,  The UnrecognisedToken()
    re_flags = re.DOTALL | re.MULTILINE

    ANCHOR_RE = re.compile(r'(?:[^[\\]|\\.|\[\^?\]?(?:\\.|[^]\\])*\])*\\[AZ]')
    BACKREF_RE = re.compile(
        r'(?:[^[\\]|\\[^0-9]|\[\^?\]?(?:\\.|[^]\\])*\])*(\\[0-9]+)')
    CAPTURE_RE = re.compile(r'[^[\\(]|\\.|\[\^?\]?(?:\\.|[^]\\])*\]|\(\?')

    def compile_tokens(self, token_registry, whitespace):
        self.registry = token_registry
        #
        # Whitespace must be a string.
        #
        if whitespace is not None:
            if not isinstance(whitespace, string_types):
                raise GrammarError("WHITESPACE must be a string")
        all_tokens = (
            token
            for token in self.registry.values()
            if isinstance(token, Token))
        key = lambda t: (to_str(t.literal), to_str(t.re))
        all_tokens = sorted(all_tokens, key=key, reverse=True)
        patterns = [token for token in all_tokens if token.re is not None]
        self.literals = {}
        self.unrecognised = next(
            (t for t in all_tokens if t.re is None and t.literal is None),
            None)
        #
        # It's amazing what unit testing turns up.  Would anybody really use
        # the inbuilt tokeniser without recognising a single token?
        #
        self.re_list = []
        self.re_groups = []
        if not patterns:
            pattern = "x(?<=y)"         # An re that never matches
        else:
            #
            # Currently Python's re module returns the first re that matches
            # when given the sequence a|b|c.  We always want it to match the
            # longest possible literal.  In Python 2.7 putting the longest
            # literals first makes that happen.
            #
            # Backreferences are allowed, but since grammar writer has no idea
            # what order we will put them in we have to renumber them.
            #
            longest = lambda token: (token.literal is not None, -len(token.re))
            ordered_patterns = []
            backref_base = 0
            for token in sorted(patterns, key=longest):
                base = backref_base
                backref_base += 1
                token_re = '(?:()(?:%s))' % token.re
                self.re_groups.append(backref_base)
                self.re_list.append(token)
                backref_matches = tuple(self.BACKREF_RE.finditer(token_re))
                for match in reversed(backref_matches):
                    backref_no = int(match.group(1)[1:], 10) + backref_base
                    token_re = "%s\\%d%s" % (
                        token_re[:match.start(1)],
                        backref_no,
                        token_re[match.end(1):])
                backref_base = base + len(self.CAPTURE_RE.sub('', token_re))
                ordered_patterns.append(token_re)
            pattern = '|'.join(ordered_patterns)
        self.regex = re.compile(pattern, self.re_flags)
        self.re_list = tuple(self.re_list)
        self.re_groups = tuple(self.re_groups)
        #
        # Gather all literals and checking there are no duplicates.
        #
        all_re = {}
        for token in all_tokens:
            #
            # Ensure the re isn't duplicated.
            #
            if token.re is not None:
                if token.re in all_re:
                    msg = "Token's %r and %r define the same re"
                    raise GrammarError(msg % (token, all_re[token.re]))
                all_re[token.re] = token
            if token.literal is not None:
                self.literals[token.literal] = token
                #
                # Ensure the literal is matched by exactly one Token.
                # This also ensures there are no duplicate literals.
                #
                matches = []
                for re_tok in patterns:
                    if self.ANCHOR_RE.match(re_tok.re):
                        continue
                    match = re.match(re_tok.re, token.literal)
                    if match is not None:
                        if (re_tok.literal is None or
                                match.group() == token.literal):
                            matches.append((re_tok, match))
                if not matches:
                    msg = "Keyword %r does not match any re"
                    raise GrammarError(msg % (token,))
                if len(matches) == 1:
                    token.owner = matches[0][0]
                else:
                    re_matches = [
                        m[0] for m in matches if m[0].literal is None]
                    if re_matches:
                        msg = (
                            "Literal token %s should be a Keyword " +
                            "as it matches re token %s")
                        res = ', '.join(str(match) for match in re_matches)
                        raise GrammarError(msg % (token, res))
                    matches = [m for m in matches if m is not token]
                    msg = "duplicate literal %r and %r"
                    raise GrammarError(msg % (matches[0][0], token,))
                if not any(m.group() == token.literal for t, m in matches):
                    msg = "Token.re %r partially matches %r of Keyword %r"
                    raise GrammarError(msg % (matches[0], match, token))

    #
    # This generator is a filter.  It takes a string generator as an argument
    # and generates tokens ready to be fed into the parser.  The strings
    # returned by the generator are assumed to be entire tokens.
    #
    def tokeniser(self, input, whitespace=None):
        #
        # pos = [position_in_stream, line_number, column_number]
        #
        def update_position(data):
            ldata = len(data)
            pos[0] += ldata
            matches = list(re.finditer("(?:\n\r?|\r\n?)", data))
            if not matches:
                pos[2] += ldata
            else:
                pos[1] += len(matches)
                pos[2] = ldata - matches[-1].end() + 1
        pos = [0, 1, 1]
        #
        # Normalise the parameters.
        #
        iterator = iter((input,) if isinstance(input, string_types) else input)
        if whitespace == "":
            is_whitespace = lambda s: False
            last_whitespace = lambda s: len(s)
        else:
            spaces = " \f\n\r\t\v" if whitespace is None else whitespace
            is_whitespace = lambda s: not s.lstrip(spaces)
            trans = string_maketrans(spaces, spaces[0] * len(spaces))
            last_whitespace = lambda s: s.translate(trans).rfind(spaces[0])
        #
        # Loop until end of the stream.
        #
        cur_tok = next(iterator, None)
        cur_isstr = isinstance(cur_tok, string_types)
        nxt_tok = next(iterator, None)
        nxt_isstr = isinstance(nxt_tok, string_types)
        while cur_tok is not None:
            buf = ""
            #
            # Loop while the current token is a string.
            #
            while cur_isstr:
                #
                # If the next token isn't a string we must parse all of the
                # input, otherwise we only parse up to the last space.  This
                # somewhat reduces the chance of tokens being truncated across
                # iterator boundaries.
                #
                last = last_whitespace(cur_tok) if nxt_isstr else len(cur_tok)
                if last == -1:
                    buf += cur_tok
                else:
                    last += len(buf)
                    buf += cur_tok
                    offset = 0
                    while offset < last:
                        match = self.regex.search(buf, offset, last)
                        if match is not None:
                            start, end = match.span()
                        else:
                            if nxt_isstr:
                                break
                            start, end = last, last
                        #
                        # The only thing that can separate one token and the
                        # next is whitespace.
                        #
                        if offset < start:
                            in_between = buf[offset:start]
                            if not is_whitespace(in_between):
                                if self.unrecognised is None:
                                    msg = (
                                        "Unrecognised token %r " +
                                        "at line %d column %d"
                                    )
                                    raise TokenError(
                                        msg % (in_between, pos[1], pos[2]),
                                        in_between, pos[0], pos[1], pos[2])
                                yield (
                                    self.unrecognised, in_between,
                                    pos[0], pos[1], pos[2])
                            update_position(in_between)
                            if start == last:
                                break
                        #
                        # Found some data that matches a token.  Identify what
                        # token it matches.
                        #
                        data = match.group()
                        try:
                            token = self.literals[data]
                        except KeyError:
                            try:
                                token = self.literals[data.lower()]
                                if token.case:
                                    token = None
                            except KeyError:
                                token = None
                            if token is None:
                                idx = match.group(*self.re_groups).index('')
                                token = self.re_list[idx]
                                if token.refine is not None:
                                    token = token.refine(self.registry, data)
                        yield token, data, pos[0], pos[1], pos[2]
                        update_position(data)
                        offset = end
                    buf = buf[offset:]
                cur_tok = nxt_tok
                cur_isstr = nxt_isstr
                nxt_tok = next(iterator, None)
                nxt_isstr = isinstance(nxt_tok, string_types)
            #
            # If we are given non-strings pass it straight on.
            #
            while cur_tok is not None and not cur_isstr:
                yield cur_tok
                cur_tok = nxt_tok
                cur_isstr = nxt_isstr
                nxt_tok = next(iterator, None)
                nxt_isstr = isinstance(nxt_tok, string_types)


#
# Meta class that does the work for a TokenRegistry.
#
class TokenRegistryMeta(type):

    def __new__(cls, name, bases, dct):
        registry = super(TokenRegistryMeta, cls).__new__(cls, name, bases, dct)
        if dct.get("__metaclass__", None) is not cls:
            registry.save_dicts()
        return registry


#
# Put your token definitions in a class that inherits from this one.
#
class TokenRegistry(dict):
    __dicts = None
    __metaclass__ = TokenRegistryMeta
    __tokeniser = None

    def __init__(self):
        for name, token_symbol in self.__class__.__dict__.items():
            if isinstance(token_symbol, TokenSymbol):
                qualified_name = "%s.%s" % (self.__class__.__name__, name)
                token_symbol.set_name(qualified_name)
                self._resolve_token_(token_symbol)

    #
    # Save the registered token's dicts, as they may be overwritten
    # by the Grammar.  This is called by the meta class, so it happens
    # before the Grammar has a chance to get it's fingers into the pie.
    #
    def save_dicts(cls):
        cls.__dicts = {}
        for token_symbol in cls.__dict__.values():
            if isinstance(token_symbol, TokenSymbol):
                cls.__dicts[token_symbol] = token_symbol.dict
                token_symbol.dict = {}
    save_dicts = classmethod(save_dicts)

    #
    # Return the dict's of all registered tokens.
    #
    def restore_dicts(cls):
        for token_symbol, token_dict in cls.__dicts.items():
            token_symbol.dict = token_dict
        del cls.__dicts
    restore_dicts = classmethod(restore_dicts)

    #
    # Resolve duplicate tokens.
    #
    def _resolve_token_(self, token_symbol):
        alias = self.get(token_symbol.name, None)
        if alias is None:
            alias = token_symbol
            super(TokenRegistry, self).__setitem__(alias.name, alias)
        if alias is not token_symbol:
            alias.merge(token_symbol)
        return alias

    #
    # Compile the re that recognises the tokens.
    #
    def compile_tokens(self, whitespace=None):
        self.__tokeniser = Tokeniser()
        self.__tokeniser.compile_tokens(self, whitespace)

    #
    # Return a generator for tokens in the grammar.
    #
    def tokeniser(self, input, whitespace=None):
        return self.__tokeniser.tokeniser(input, whitespace)

    def __setitem__(self, key, value):
        raise NotImplementedError()

    def __delitem__(self, key):
        raise NotImplementedError()


TokenRegistry = python3_metaclass(TokenRegistry)


#
# A Token in the grammar.  An instance of a TokenSymbol() defines one kind
# of token.
#
class TokenSymbol(Symbol):
    __slots__ = ('name', 'named',)

    def __init__(self, name):
        super(TokenSymbol, self).__init__()
        self.name = name
        self.named = False
        self.first_set = frozenset((self,))

    def resolve_symbol(self, name, rules, token_definitions):
        return token_definitions._resolve_token_(self)

    #
    # Absorb another token definition into this one.
    #
    def merge(self, other):
        msg = "Token %r doesn't support merging with token %r"
        raise GrammarError(msg % (self, other))

    def compile_symbol(self, comp):
        return [(self,)]

    #
    # Set the name of this token.
    #
    def set_name(self, name):
        if self.name is None:
            self.name = name
        elif self.name != name:
            msg = "Can not rename token %r to %r" % (self.name, name)
            raise GrammarError(msg)
        self.named = True

    def __repr__(self):
        return str(self)

    def __str__(self):
        return str(self.name)

    #
    # Given a tuple returned by the tokeniser, return an English
    # description of where we are in the input stream.
    #
    def position(self, token_tuple):
        return None


#
# MetaToken's are used internally by the Parser.  They are re-usable
# by multiple grammar's.
#
class MetaToken(TokenSymbol):
    __slots__ = ()

    def __init__(self, name):
        super(MetaToken, self).__init__(name)

    def __repr__(self):
        return str(self.name)

    def __str__(self):
        return self.name

    def position(self, token_tuple):
        return self.name


#
# A user generated token.
#
class UserToken(TokenSymbol):
    __slots__ = ()

    def __init__(self, name=None):
        super(UserToken, self).__init__(name)

    def resolve_symbol(self, name, rules, token_definitions):
        if self.name is None:
            msg = "A %s must be assigned a name using a %s" % (
                self.__class__.__name__, TokenRegistry.__name__)
            raise GrammarError(msg)
        return super(UserToken, self).resolve_symbol(
            name, rules, token_definitions)

    def merge(self, other):
        if not isinstance(other, UserToken):
            super(UserToken, self).merge(other)


#
# A Token() built by the inbuilt tokeniser.  A token comes in two
# varieties:
#
#   - A token defined by a regular expression.
#   - A keyword, which is a special purposed token.
#
class Token(TokenSymbol):
    _RE = re
    KEYWORD = object()
    UNRECOGNISED = object()
    __slots__ = ('case', 'literal', 'owner', 're', 'refine')

    def __init__(
            self, literal=None, re=None, case=None, kind=None, refine=None):
        if kind is self.KEYWORD:
            if re is not None:
                raise GrammarError("A keyword must not have an re")
            if refine is not None:
                raise GrammarError("A keyword can not be refined")
            if literal is None:
                raise GrammarError("A keyword must have a literal")
        elif kind is self.UNRECOGNISED:
            if literal is not None or re is not None:
                msg = "The UnrecognisedToken can't have a literal or re"
                raise GrammarError(msg)
        elif kind is not None:
            raise GrammarError("Unrecognised Token kind %r" % (kind,))
        else:
            if literal is None and re is None:
                raise GrammarError("A Token must have a literal or an re")
            if literal is not None:
                if re is not None:
                    msg = "A Token can't have both a literal and a re"
                    raise GrammarError(msg)
                if refine is not None:
                    raise GrammarError("A literal can't be refined")
        self.literal = literal
        self.re = re
        self.named = False
        super(Token, self).__init__(str(self))
        self.case = case if case is not None else True
        self.refine = refine
        self.owner = self
        if kind is not None:
            pass
        elif re is not None:
            self._RE.compile(re)
        else:
            self.re = self._RE.escape(self.literal)
            if not self.case and self.literal.lower() != self.literal.upper():
                def either_case(match):
                    char = match.group()
                    return "[%c%c]" % (char.upper(), char.lower())
                self.re = self._RE.sub("[a-zA-Z]", either_case, self.re)

    def __repr__(self):
        if self.literal is not None:
            result = repr(self.literal)
        elif self.re is not None:
            result = self.repr_re()
        else:
            result = "%s()" % (UnrecognisedToken.__name__,)
        if self.named:
            return '%s=%s' % (self, result,)
        return result

    def __str__(self):
        if self.named:
            return super(Token, self).__str__()
        if self.literal:
            return repr(self.literal)
        if self.re is not None:
            return self.repr_re()
        return '<unrecognised_token>'

    def repr_re(self):
        return "/%s/" % (repr(self.re)[1:-1].replace("/", "\\/"),)

    def merge(self, other):
        #
        # We are only merge with like.
        #
        if not isinstance(other, Token):
            TokenSymbol.merge(self, other)
        if self.refine is None:
            self.refine = other.refine
        elif other.refine is not None and other.refine != self.refine:
            msg = "Token %r defined with conflicting refine's %r and %r"
            raise GrammarError(msg % (self.name, self.refine, other.refine))

    #
    # Set the name of this token.
    #
    def set_name(self, name):
        if self.named and self.name != name:
            msg = "Can not rename token %r to %r" % (self.name, name)
            raise GrammarError(msg)
        self.name = name
        self.named = True

    #
    # Our tokeniser puts the line and column in the tuple.
    #
    def position(self, token_tuple):
        if len(token_tuple) < 5 or None in token_tuple[3:5]:
            return super(Token, self).position(token_tuple)
        return "line %d column %d" % token_tuple[3:5]


#
# The Unrecognised Token.
#
def UnrecognisedToken():
    return Token(kind=Token.UNRECOGNISED)


#
# A Keyword is literal without a regexp, ie it must match an existing regexp.
#
def Keyword(literal, case=None):
    return Token(literal, case=case, kind=Token.KEYWORD)


#
# How we handle non-Symbol types in Symbol expression.
#
Symbol.CAST = {
    str: Token,
    tuple: lambda t: Prio(*t),
}
if sys.version_info < (3,):
    Symbol.CAST[unicode] = Token


class Production(object):
    __slots__ = ("lhs", "rhs")

    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = tuple(rhs)

    def __repr__(self):
        return (
            "%s = %s" %
            (self.lhs, " ".join(str(elm) for elm in self.rhs),))


#
# Parser Constructor.
#
class GrammarMeta(type):

    def __new__(cls, name, bases, dct):
        if "_parser_" in dct:
            raise GrammarError("_parser_ is reserved in Gramma's.")
        if dct.get("__metaclass__", None) is not cls:
            dct["_parser_"] = Parser(name, dct)
        return super(GrammarMeta, cls).__new__(cls, name, bases, dct)


#
# The base class for Parsers.
#
class Grammar(object):
    __metaclass__ = GrammarMeta

    def compile_grammar(cls):
        compile_grammar(cls)
    compile_grammar = classmethod(compile_grammar)

    def epoch_symbol(cls):
        return epoch_symbol(cls)
    epoch_symbol = classmethod(epoch_symbol)

    def parse(cls, input, tree_factory=None, on_error=None, log=None):
        return parse(cls, input, tree_factory, on_error, log)
    parse = classmethod(parse)

    def pre_compile_grammar(cls, pre_compiled=None):
        return pre_compile_grammar(cls, pre_compiled)
    pre_compile_grammar = classmethod(pre_compile_grammar)

    def repr_grammar(cls):
        return repr_grammar(cls)
    repr_grammar = classmethod(repr_grammar)

    def repr_parse_table(cls, state=None):
        return repr_parse_table(cls, state)
    repr_parse_table = classmethod(repr_parse_table)

    def repr_parse_tree(cls, tree, indent=None):
        return repr_parse_tree(tree, indent)
    repr_parse_tree = classmethod(repr_parse_tree)

    def repr_productions(cls):
        return repr_productions(cls)
    repr_productions = classmethod(repr_productions)

    def unused_rules(cls):
        return unused_rules(cls)
    unused_rules = classmethod(unused_rules)


Grammar = python3_metaclass(Grammar)


def compile_grammar(grammar):
    grammar._parser_.compile_grammar()


def epoch_symbol(grammar):
    return grammar._parser_.epoch_symbol


def parse(grammar, input, tree_factory=None, on_error=None, log=None):
    return grammar._parser_.parse(input, tree_factory, on_error, log)


def pre_compile_grammar(grammar, pre_compiled=None):
    return grammar._parser_.pre_compile_grammar(grammar, pre_compiled)


def repr_grammar(grammar):
    return grammar._parser_.repr_grammar()


def repr_parse_table(grammar, state=None):
    return grammar._parser_.repr_parse_table(state)


def repr_productions(grammar):
    return grammar._parser_.repr_productions()


def unused_rules(grammar):
    return grammar._parser_.unused_symbols


def repr_parse_tree(tree, indent=None):
    def indent_tree(tree, padding):
        #
        # Append tokens and empty productions to the current line.
        #
        def extend_line(line, prod):
            while True:
                while prod and isinstance(prod[0][0], TokenSymbol):
                    line.append(repr_token(prod.popleft()))
                if not prod or len(prod[0]) != 1:
                    break
                line.append("(%s)" % (prod.popleft()[0],))
        repr_token = (
            lambda t: repr(t[1]) if isinstance(t[0], Token) else str(t[0]))
        #
        # Were we passed a token?
        #
        if isinstance(tree[0], TokenSymbol):
            return [repr_token(tree)]
        #
        # List singleton productions (ie productions of the form rule1 = rule2)
        # on the same line.
        #
        line, result = [], []
        nesting = 1
        while len(tree) == 2 and not isinstance(tree[1][0], TokenSymbol):
            line.append("(%s" % (tree[0],))
            nesting += 1
            tree = tree[1]
        line.append("(%s" % (tree[0],))
        #
        # If the remainder of the symbols are tokens just list them as well.
        #
        prod = collections.deque(tree[1:])
        extend_line(line, prod)
        if not prod:
            result.append('%s%s' % (padding, ' '.join(line)))
        else:
            #
            # If we have rule1 = ... ^ rule2, then list them as:
            #
            #   (rule1 ... rule2
            #      rule2-child0
            #      ...)
            #
            # If we have rule1 = ... ^ rule2 ... then list them as:
            #
            #   (rule1 ...
            #     (rule2
            #       ...)
            #     ...)
            #
            if len(prod) > 1:
                result.append('%s%s' % (padding, ' '.join(line)))
            elif (len(prod) == 1 and
                    all(isinstance(t[0], TokenSymbol) for t in prod[0][1:])):
                last = prod.popleft()
                line.append("(%s" % last[0])
                line.extend(repr_token(t) for t in last[1:])
                line[-1] += ")"
                result.append('%s%s' % (padding, ' '.join(line)))
            else:
                line.append('(%s' % (prod[0][0],))
                result.append('%s%s' % (padding, ' '.join(line)))
                prod = collections.deque(prod[0][1:])
                if len(prod[0]) == 1 or isinstance(prod[0][0], TokenSymbol):
                    line = []
                    extend_line(line, prod)
                    result.append('%s%s' % (padding + indent, ' '.join(line)))
            while prod:
                result.extend(indent_tree(prod.popleft(), padding + indent))
                line = [result[-1]]
                extend_line(line, prod)
                result[-1] = ' '.join(line)
        result[-1] += ')' * nesting
        return result
    eol = ' ' if indent is False else '\n'
    indent = "  " if indent is None else indent or ""
    return eol.join(indent_tree(tree, ""))

# vim: set shiftwidth=4 expandtab softtabstop=8 :
