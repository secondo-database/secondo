export const ENVIRONMENT: string = process.env.NODE_ENV || "local";

import * as compression from "compression";
import * as expressValidator from "express-validator";
import { json as bodyparser_json, urlencoded as bodyparser_urlencoded } from "body-parser";
import { xframe as lusca_xframe, xssProtection as lusca_xssProtection } from "lusca";
import * as path from "path";
import * as cookieParser from "cookie-parser";
import { loadSettings } from "./common/SettingsHelper";
import { Logger } from "winston";
import { Server as http_server, createServer as createHttpServer } from "http";
import { Server as https_server, createServer as createHttpsServer } from "https";
import { readFileSync } from "fs";
import { Settings } from "./settings/Settings";
import { isMaster as cluster_isMaster, on as cluster_on, fork as cluster_fork, Worker as cluster_Worker } from "cluster";
import * as express from "express";
import { buildRouter } from "./routes";
import { ConfigureHeadersForApp } from "./middleware/Headers";

const logger: Logger = require("./common/logger")(module);

export class Application {
    private asCluster: boolean = true;
    private restartIfCrashed: boolean = true;
    private numCPUs: number = 1;
    private static appSettings: Settings = undefined;

    constructor(_asCluster?: boolean, _restartIfCrashed?: boolean) {
        this.asCluster = _asCluster !== undefined ? _asCluster : this.asCluster;
        this.restartIfCrashed = _restartIfCrashed !== undefined ? _restartIfCrashed : this.restartIfCrashed;
    }

    public static getSettings(): Settings {
        return this.appSettings;
    }

    public startApplication() {

        if (this.asCluster) {
            this.numCPUs = require("os").cpus().length;
        }

        if (cluster_isMaster) {
            logger.info(`Master ${process.pid} is running`);

            for (let i = 0; i < this.numCPUs; i++) {
                cluster_fork();
            }

            cluster_on("exit", (worker: cluster_Worker, code: number, signal: string) => {
                logger.info(`worker ${worker.process.pid} died`);
                if (this.restartIfCrashed) {
                    cluster_fork();
                }
            });
        } else {
            this.setUpAndStartApplication().then((started: boolean) => {
                logger.info("Application started");
            }).catch((notStarted: any) => {
                logger.error("Error while starting application: " + notStarted);
            });
            logger.info(`Worker ${process.pid} started`);
        }
    }

    private setUpAndStartApplication(): Promise<boolean> {
        return new Promise<boolean>((resolve: any, reject: any) => {
            const credentialsLoadedPromise: Promise<Settings> = loadSettings();

            const setUpExpressAppPromise: Promise<express.Application> = credentialsLoadedPromise.then((settings: Settings) => {
                return this.setUpExpressApp(settings);
            });

            Promise.all([
                credentialsLoadedPromise,
                setUpExpressAppPromise,
            ]).then((fulfilled: [Settings, express.Application]) => {
                this.createAndStartHttpServer(fulfilled[1]);
                this.createAndStartHttpsServer(fulfilled[1]);
                resolve(true);
                return;
            }).catch((unfulfilled: any) => {
                logger.error("Error while starting server: " + unfulfilled);
                reject();
                return;
            });

        });
    }

    private setUpExpressApp(settings: Settings): Promise<express.Application> {
        return new Promise<express.Application>((resolve: any, reject: any) => {
            Application.appSettings = settings;
            const app: express.Application = express();

            app.set("port", process.env.PORT || 3000);
            app.set("view engine", "ejs");
            app.use(compression());
            app.use(bodyparser_json());
            app.use(bodyparser_urlencoded({ extended: true }));
            app.use(expressValidator());
            app.use(lusca_xframe("SAMEORIGIN"));
            app.use(lusca_xssProtection(true));
            app.use(express.static(path.join(__dirname, "public"), { maxAge: 31557600000 }));
            app.use(cookieParser());
            app.use(ConfigureHeadersForApp.configureHeaders);

            buildRouter(app);

            if (ENVIRONMENT !== "local") {
                app.use(function (err: any, req: express.Request, res: express.Response, next: express.NextFunction) {
                    logger.error(err);
                    res.status(500).send({ status: 500, message: "Ein interner Fehler ist aufgetreten. Wir bitten um Entschuldigung." });
                });
            }

            app.use(function (req: express.Request, res: express.Response, next: express.NextFunction) {
                res.status(404);
                if (req.accepts("json")) {
                    res.send({ error: "Not found" });
                    return;
                }
            });
            resolve(app);
        });
    }

    private createAndStartHttpServer(app: express.Application) {
        const server: http_server = createHttpServer(app);
        if (ENVIRONMENT === undefined || ENVIRONMENT === "local") {
            server.listen(app.get("port"), () => {
                logger.info(("App is running at http://localhost:%d in %s mode"), app.get("port"), app.get("env"));
                logger.info("Press CTRL-C to stop\n");
            });
        }
    }

    private createAndStartHttpsServer(app: express.Application) {
        if (process.env.USE_SSL && process.env.USE_SSL === "true") {
            // Try to load certificates. If no certificates are found https will not start
            try {
                logger.debug("Will use ssl");
                app.set("port_ssh", process.env.PORT_SSH || 3343);
                const serverHTTPS: https_server = createHttpsServer({
                    //ca: fs.readFileSync(process.cwd() + "/certificates/ca.pem"),
                    cert: readFileSync(process.cwd() + "/certificates/cer.pem"),
                    key: readFileSync(process.cwd() + "/certificates/key.key")
                }, app);
                // Start https server
                serverHTTPS.listen(app.get("port_ssh"), () => {
                    logger.info(("App is running at https://localhost:%d in %s mode"), app.get("port_ssh"), app.get("env"));
                    logger.info("Press CTRL-C to stop\n");
                });
            } catch {
                logger.error("SSL-Server could not be started. No certificates found");
            }
        }
    }
}

process.on("uncaughtException", function (err) {
    logger.error((new Date).toUTCString() + " uncaughtException:", err.message);
    logger.error(err.stack);
    process.exit(1);
});