import { Application, Router, IRouterMatcher } from "express";
import { RouteInterface } from "./RouteInterface";
import { journalRoutes } from "./journal";
import { workerStatusRoutes } from "./workerstatus";


export const routes: RouteInterface[] = [
    ...journalRoutes(),
    ...workerStatusRoutes()
];

const BASE_URL: string = "/v1";

export function buildRouter(app: Application) {
    const router = Router();

    routes.forEach((r: RouteInterface) => {
        const method: (keyof Router) = (r.method.toLowerCase() as any);
        (router[method] as IRouterMatcher<Router>)(r.path, [...r.authenticators, ...r.validators], r.handler);
        app.use(BASE_URL, router);
    });
}