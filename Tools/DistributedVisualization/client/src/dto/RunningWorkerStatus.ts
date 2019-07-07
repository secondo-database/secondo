import { WorkerStatusDTO } from "./WorkerStatus";

export interface RunningWorkerStatusDTO {
    workerStatus: {[key: string]: WorkerStatusDTO};
}
