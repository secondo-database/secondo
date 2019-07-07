import * as fs from "fs";
import { Logger } from "winston";
import { Settings } from "../settings/Settings";

const environment: string = process.env.NODE_ENV || "development";

/**
 * Provides functions, DTOs and methodes to load configuration into memory
 */
const logger: Logger = require("./logger")(module);
/**
 * Data transfer objects for credentials
 */

let settings: Settings;

export async function loadSettings(): Promise<Settings> {
    const errorMessage: string = "Error occured while reading credentials";
    return new Promise<Settings>((resolve: any, reject: any) => {
        if (settings === undefined) {
            logger.debug("Will read credentials for: " + environment);
            fs.readFile(process.cwd() + "/config/" + environment + ".json", (err: NodeJS.ErrnoException, data: Buffer) => {
                if (err) {
                    logger.error("Error occurred while reading settings: " + errorMessage);
                    reject(errorMessage);
                    return;
                }
                const settingsJSON: any = data.toString();
                settings = JSON.parse(settingsJSON);
                resolve(settings);
                return;
            });
        } else {
            resolve(settings);
            return;
        }
    });
}