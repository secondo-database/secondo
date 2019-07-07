import { JournalDTO } from "../dto/Journal";
import { Application } from "../app";
import { readFile } from "fs";
import { Logger } from "winston";

const logger: Logger = require("../common/logger")(module);

export class JournalService {
    public static getJournal(): Promise<JournalDTO[]> {
        const journalFileLocation = Application.getSettings().distributed_algebra.log_location_absolute +
            "/" + Application.getSettings().distributed_algebra.journal_file_name;
        const errorMessage: string = "Error occured while reading Journal file: ";
        return new Promise<JournalDTO[]>((resolve: any, reject: any) => {
            readFile(journalFileLocation, (err: NodeJS.ErrnoException, data: Buffer) => {
                if (err) {
                    logger.error(errorMessage + err);
                    reject(errorMessage + err);
                    return;
                }
                try {
                    const journal: JournalDTO[] = this.parseJournal(data);
                    resolve(journal);
                    return;
                }
                catch (err) {
                    reject(errorMessage + err);
                    return;
                }

            });
        });
    }

    private static parseJournal(data: Buffer): JournalDTO[] {
        const rawJournal: any = data.toString();
        const rawJournalJSON: any = JSON.parse(rawJournal);
        const keys: string[] = Object.keys(JSON.parse(rawJournal));
        const journal: JournalDTO[] = new Array<JournalDTO>();
        for (const key of keys) {
            const journalDTO: JournalDTO = rawJournalJSON[key];
            if (Application.getSettings().general.show_only_running_jobs && journalDTO.status !== "running") {
                continue;
            }
            journalDTO.job_id = key;
            journalDTO.tree = JSON.parse(rawJournalJSON[key].tree);
            journal.push(journalDTO);
        }
        return journal;
    }
}