import { NextFunction, Request, Response } from "express";
import { JournalService } from "../service/Journal";
import { JournalDTO } from "../dto/Journal";

export class JournalController {

    public static getJournal(req: Request, res: Response, next: NextFunction) {
        JournalService.getJournal().then((journal: JournalDTO[]) => {
            res.status(200).json(journal);
        }).catch((err: any) => {
            res.status(500).json({ "message": err });
        });
    }
}
