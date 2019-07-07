import { NextFunction, Request, Response } from "express";
import { WorkerStatusService } from "../service/WorkerStatus";
import { WorkerStatusDTO } from "../dto/WorkerStatus";


export class WorkerStatusController {
    public static getWorkerStatus(req: Request, res: Response, next: NextFunction) {
        const file: string = req.query["file"];
        if (file === undefined) {
            res.status(400).json({ "message": "Query parameter file has to be provided" });
        }
        WorkerStatusService.getWorkerStatus(file).then((workerStatus: WorkerStatusDTO) => {
            res.status(200).json(workerStatus);
        }).catch((err: any) => {
            res.status(500).json({ "message": err });
        });
    }
}
