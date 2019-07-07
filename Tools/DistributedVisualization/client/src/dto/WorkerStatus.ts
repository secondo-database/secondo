import { WorkerDTO } from "./Worker";

export interface WorkerStatusDTO {
    workers: WorkerDTO[];
    job: string;
    file: string;
    node: string;
}
