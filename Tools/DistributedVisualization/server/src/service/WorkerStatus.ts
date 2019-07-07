import { Logger } from "winston";
import { Application } from "../app";
import { readFile } from "fs";
import { WorkerStatusDTO } from "../dto/WorkerStatus";
import { WorkerDTO } from "../dto/Worker";

const logger: Logger = require("../common/logger")(module);
export class WorkerStatusService {
    public static getWorkerStatus(file: string): Promise<WorkerStatusDTO> {
        const workerStatusFileLocation: string = Application.getSettings().distributed_algebra.log_localtion_absolute + "/" + file;
        const errorMessage: string = "Error occured while reading WorkerStatus file: ";
        return new Promise<WorkerStatusDTO>((resolve: any, reject: any) => {
            readFile(workerStatusFileLocation, (err: NodeJS.ErrnoException, data: Buffer) => {
                if (err) {
                    logger.error(errorMessage + err);
                    reject(errorMessage + err);
                    return;
                }
                try {
                    const workerStatus: WorkerStatusDTO = this.parseWorkerStatus(data);
                    resolve(workerStatus);
                    return;
                }
                catch (err) {
                    reject(errorMessage + err);
                    return;
                }

            });
        });
    }

    private static parseWorkerStatus(data: Buffer): WorkerStatusDTO {
        const workerStatus: WorkerStatusDTO = {} as WorkerStatusDTO;
        const workers: WorkerDTO[] = new Array();
        workerStatus.workers = workers;
        const rawWorkerStatus: any = data.toString();
        const rawWorkerStatusJSON: any = JSON.parse(rawWorkerStatus);
        const keys: string[] = Object.keys(JSON.parse(rawWorkerStatus));
        for (const key of keys) {
            if (key === "job") {
                    workerStatus.job = rawWorkerStatusJSON.job.id;
                }
            else if (key === "node") {
                workerStatus.node = rawWorkerStatusJSON.node;
            }
                else {
                    const worker: WorkerDTO = {} as WorkerDTO;
                    workerStatus.workers;
                    worker.id = key;
                    worker.progress = rawWorkerStatusJSON[key].progress;
                    worker.status = rawWorkerStatusJSON[key].status;
                    worker.runningProgress = rawWorkerStatusJSON[key].runningProgress;
                    worker.slots = rawWorkerStatusJSON[key].slots.sort(function(a: string, b: string){
                        if(a === "running") {
                            return 1;
                        }
                        if (a === "created") {
                            return 1;
                        }
                        if (a === "finished") {
                            return -1;
                        }
                    });
                    worker.started = rawWorkerStatusJSON[key].started;
                    worker.finished = rawWorkerStatusJSON[key].finished;
                    workers.push(worker);
                }
        }
        return workerStatus;
    }

}