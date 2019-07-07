import { WorkerDTO } from "./Worker";

export interface WorkerStatusDTO {
    workers: WorkerDTO[];
    job: string;
    node: string;
}