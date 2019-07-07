import { TreeDTO } from "./Tree";

export interface JournalDTO {
    job_id: string;
    current_file: string;
    files: string[];
    finished: string;
    id: string;
    numOfWorkers: number;
    query: string;
    nestedList: string;
    started: string;
    status: string;
    tree: { [key: string]: TreeDTO };
    observableNodesIds: number[];
    nowWorkingOnNodeId: number;
    readyNodesIds: number[];
    numOfSlotsPerWorker: number;
    nodeOperatorMapping: { [key: string]: string};
    nodeArgumentsMapping: { [key: string]: string};
}
