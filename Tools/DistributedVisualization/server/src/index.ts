import { Application } from "./app";

startApplication();

function startApplication() {
    const app: Application = new Application(false, true);
    app.startApplication();
}