import { WorkerStatusController } from "../controllers/WorkerStatus";
import { RouteInterface } from "./RouteInterface";

const WORKERSTATUS_BASE: string = "workerstatus";
export function workerStatusRoutes(): RouteInterface[] {
  return [
    {
      method: 'GET',
      path: `/${WORKERSTATUS_BASE}`,
      validators: [],
      authenticators: [],
      handler: WorkerStatusController.getWorkerStatus,
    }
  ];
} 