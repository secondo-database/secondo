import { TreeDTO } from "@/dto/Tree";
import { TreeForDisplayDTO } from "@/dto/TreeForDisplay";
import { JournalDTO } from "@/dto/Journal";
import store from "@/store";
import { SET_TREE_FOR_DISPLAY, SET_SIMPLIFIED_ARGS } from "@/store/mutation_types";
import { SimplifiedArgsDTO } from "@/dto/SimplifiedArgsDTO";

const simplifiedArgs: SimplifiedArgsDTO = { result: {}, args: {} } as SimplifiedArgsDTO;

export class TreeService {
    public static prepareTreeForDisplay(journal: JournalDTO) {
        const tree: { [key: string]: TreeDTO } = journal.tree;
        const keys: string[] = Object.keys(tree);
        const treeForDisplay: TreeForDisplayDTO = {} as TreeForDisplayDTO;
        treeForDisplay.name = keys[0];
        treeForDisplay.operator = tree[keys[0]].operator;
        const keyAsNumber: number = Number.parseInt(keys[0], 10);
        if (journal.observableNodesIds.includes(keyAsNumber)) {
            treeForDisplay.observable = true;
            treeForDisplay.children = new Array();
            if (journal.nowWorkingOnNodeId === keyAsNumber) {
                treeForDisplay.nowWorkingOn = true;
            } else if (journal.readyNodesIds.includes(keyAsNumber)) {
                treeForDisplay.ready = true;
            }

        }

        treeForDisplay.children = this.parseTreeChildren(tree, keys[0], journal);
        this.replaceResults(treeForDisplay, journal);
        this.enrichTree(treeForDisplay, journal);
        store.commit(SET_TREE_FOR_DISPLAY, treeForDisplay);
        store.commit(SET_SIMPLIFIED_ARGS, simplifiedArgs);
    }

    public static getArguments(journal: JournalDTO, node: string): string {
        const op: string = journal.nodeOperatorMapping[node];
        if (this.isDmap(op)) {
            return this.getDmapArguments(journal.nodeArgumentsMapping[node], op);
        }
        return journal.nodeArgumentsMapping[node];
    }

    public static getSimplifiedArguments(node: string): string {
        const op: string = store.getters.simplifiedArgs.args[node];
        return op;
    }

    public static getSimplifiedResult(node: string): string {
        const op: string = store.getters.simplifiedArgs.result[node];
        return op;
    }

    private static parseTreeChildren(
        tree: { [key: string]: TreeDTO },
        key: string, journal: JournalDTO): TreeForDisplayDTO[] {

        const observableNodesIds: number[] = journal.observableNodesIds;
        const nowWorkingOnNodeId: number = journal.nowWorkingOnNodeId;
        const readyNodesIds: number[] = journal.readyNodesIds;
        if (tree[key].sons === undefined) {
            return [];
        }
        const children: TreeForDisplayDTO[] = new Array();
        const sons: number[] = tree[key].sons;
        for (const son of sons) {
            const treeForDisplay: TreeForDisplayDTO = {} as TreeForDisplayDTO;
            treeForDisplay.name = son.toString();
            treeForDisplay.operator = tree[son].operator;
            treeForDisplay.children = this.parseTreeChildren(tree, son.toString(), journal);

            if (observableNodesIds.includes(son)) {
                treeForDisplay.observable = true;
                if (nowWorkingOnNodeId === son) {
                    treeForDisplay.nowWorkingOn = true;
                } else if (readyNodesIds.includes(son)) {
                    treeForDisplay.ready = true;
                }
                children.push(treeForDisplay);
            } else if (this.hasObservableChildren(tree, son.toString(), observableNodesIds)) {
                children.push(treeForDisplay);
            }
        }
        return children;
    }

    private static hasObservableChildren(
        tree: { [key: string]: TreeDTO }, key: string,
        observableNodesIds: number[]): boolean {

        let hasObservableChildren: boolean = false;
        if (tree[key].sons === undefined) {
            return hasObservableChildren;
        }
        const sons: number[] = tree[key].sons;
        for (const son of sons) {
            if (observableNodesIds.includes(son)) {
                hasObservableChildren = true;
                break;
            }
        }
        return hasObservableChildren;
    }

    private static replaceResults(treeForDisplay: TreeForDisplayDTO, journal: JournalDTO) {
        if (treeForDisplay.children === undefined) {
            return;
        }
        let splitTreeArgs: string[] = [];
        if (journal.nodeArgumentsMapping[treeForDisplay.name] !== undefined) {
            splitTreeArgs = journal.nodeArgumentsMapping[treeForDisplay.name].split(" : ");


            splitTreeArgs.forEach((value: string) => {
                value = value.trimLeft();
                value = value.trimRight();
            });
        }

        for (const elem of treeForDisplay.children) {
            if (journal.nodeArgumentsMapping[elem.name] === undefined) {
                return;
            }
            let result: string = "";
            const splitArgs: string[] = journal.nodeArgumentsMapping[elem.name].split(" : ");
            splitArgs.forEach((value: string) => {
                value = value.trimLeft();
                value = value.trimRight();
                value = "(" + elem.operator + " " + value + ")";
            });

            const childArg: string = "(" + elem.operator + " " + splitArgs.join(" ") + ")";
            const splitTreeArgsNew: string[] = new Array();

            splitTreeArgs.forEach((value: string) => {
                if (value === childArg) {
                    result = "R" + elem.name;
                    elem.result = result;
                    value = result;
                }
                splitTreeArgsNew.push(value);
            });

            if (simplifiedArgs.args[treeForDisplay.name] !== undefined) {
                const simplifiedArgsArray: string[] = simplifiedArgs.args[treeForDisplay.name].split(", ");
                const union: string[] = [...simplifiedArgsArray, ...splitTreeArgsNew];
                const diff: string[] = union.filter((a) => !splitTreeArgs.includes(a));
                const diffRest: string[] =
                    [...splitTreeArgs.filter((a) => splitTreeArgsNew.includes(a))
                        .filter((b) => simplifiedArgsArray.includes(b))];
                if ((diff.length + diffRest.length) <= splitTreeArgs.length) {
                    simplifiedArgs.args[treeForDisplay.name] = [...diff, ...diffRest].join(", ");
                }

            } else {
                simplifiedArgs.args[treeForDisplay.name] = splitTreeArgsNew.join(", ");
            }

            simplifiedArgs.result[elem.name] = result;
            this.replaceResults(elem, journal);
        }
        if (treeForDisplay.children === undefined || treeForDisplay.children.length === 0) {
            simplifiedArgs.args[treeForDisplay.name] = splitTreeArgs.join(", ");
            simplifiedArgs.result[treeForDisplay.name] = "R" + treeForDisplay.name;
        }
    }

    private static enrichTree(treeForDisplay: TreeForDisplayDTO, journal: JournalDTO) {
        if (treeForDisplay.children === undefined) {
            return;
        }
        for (const elem of treeForDisplay.children) {
            if (elem.children && elem.children.length > 0) {
                this.enrichTree(elem, journal);
            } else {
                if (elem.operator && this.isDmap(elem.operator)) {
                    elem.children = new Array();
                    const child: TreeForDisplayDTO =
                        this.createChild("-1",
                            this.getDmapArguments(journal.nodeArgumentsMapping[elem.name], elem.operator),
                            false,
                            elem.ready,
                            true);
                    elem.children.push(child);
                }
            }

        }
    }

    private static isDmap(op: string): boolean {
        if (op === "dmap"
            || op === "dmap2"
            || op === "dmap3") {
            return true;
        }
        return false;
    }

    private static getDmapArguments(rawArgs: string, op: string): string {
        if (rawArgs === undefined) {
            return "";
        }
        const args: string[] = rawArgs.split(" : ");
        let nArgs = "";

        switch (op) {
            case "dmap":
                nArgs = args[0];
                break;
            case "dmap2":
                nArgs = args[0].trimRight() + ", " + args[1];
                break;
            case "dmap3":
                nArgs = args[0].trimRight() + ", " + args[1].trimRight() + ", " + args[2];
                break;
        }
        return nArgs;
    }

    private static createChild(
        name: string,
        operator: string,
        isObservable: boolean,
        isReady: boolean | undefined,
        operand: boolean): TreeForDisplayDTO {

        const treeForDisplay: TreeForDisplayDTO = {} as TreeForDisplayDTO;
        treeForDisplay.name = name;
        treeForDisplay.operator = operator;
        treeForDisplay.observable = isObservable;
        treeForDisplay.ready = isReady;
        treeForDisplay.operand = operand;
        return treeForDisplay;
    }
}
