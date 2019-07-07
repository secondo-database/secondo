export interface WorkerDTO {
    status: string;
    progress: number;
    id: string;
    slots: string[];
    runningProgress: number;
    started: string;
    finished: string;
}