import axios, { AxiosResponse } from "axios";
import config from "../../config/default";
import {
  SET_GLOBAL_LOADING,
  SET_GLOBAL_ERROR,
  CLEAR_GLOBAL_ERROR,
  SET_WORKERSTATUS_FILES,
  UPDATE_RUNNING_WORKERSTATUS,
  RESET_RUNNING_WORKERSTATUS,
  LOAD_WORKERSTATUS_ONCE,
} from "../mutation_types";
import { WorkerStatusDTO } from "@/dto/WorkerStatus";
import { RunningWorkerStatusDTO } from "@/dto/RunningWorkerStatus";
import store from "..";
import Vue from "vue";

const apiUrl: string = config.base_url + config.api_version + config.workerstatus_path;

export default {
  state: {
    runningWorkerStatus: ({ workerStatus: {} } as RunningWorkerStatusDTO),
    runningWorkerStatusFiles: ([] as string[]),
    loadWorkerStatusOnce: true,
  },

  mutations: {
    [UPDATE_RUNNING_WORKERSTATUS](state: any, workerStatus: WorkerStatusDTO): void {
      Vue.set(state.runningWorkerStatus.workerStatus, workerStatus.node, workerStatus);

    },
    [SET_WORKERSTATUS_FILES](state: any): void {
      const runningWorkerStatusFiles: string[] = new Array();

      for (const journalItem of store.getters.journalItems) {
        if ((state.loadWorkerStatusOnce && journalItem.status === "finished") ||
            journalItem.status !== "finished" ||
              (journalItem.finished !== ""
                && (Math.floor(new Date().getTime() / 1000) - Number.parseInt(journalItem.finished, 10) < 100))) {
          for (const file of journalItem.files) {
            runningWorkerStatusFiles.push(file);
          }
        }
      }
      store.commit(LOAD_WORKERSTATUS_ONCE, false);
      state.runningWorkerStatusFiles = runningWorkerStatusFiles;
    },
    [RESET_RUNNING_WORKERSTATUS](state: any): void {
      const runningWorkerStatusFiles: string[] = new Array();
      state.runningWorkerStatusFiles = runningWorkerStatusFiles;
    },
    [LOAD_WORKERSTATUS_ONCE](state: any, value: boolean): void {
      state.loadWorkerStatusOnce = value;
    },
  },

  actions: {
    pollWorkerStatusAction({ commit, dispatch }: any): void {
      if (store.getters.runningWorkerStatusFiles.length === 0) {
        return;
      }
      for (const file of store.getters.runningWorkerStatusFiles) {
        const url: string = apiUrl
          + "?file="
          + file;
        axios.get(url, {
          headers: {
            //
          },
        }).
          then((getWorkerStatusResponse: AxiosResponse) => {
            commit(UPDATE_RUNNING_WORKERSTATUS, getWorkerStatusResponse.data);
            commit(SET_GLOBAL_LOADING, false);
            commit(CLEAR_GLOBAL_ERROR);
          }).catch((getWorkerStatusError: any) => {
            if (getWorkerStatusError.response !== undefined) {
              const errMessage = getWorkerStatusError.response.data.message;
              commit(SET_GLOBAL_LOADING, false);
              commit(SET_GLOBAL_ERROR, errMessage);
            } else {
              const errMessage = "Server not available";
              commit(SET_GLOBAL_LOADING, false);
              commit(SET_GLOBAL_ERROR, errMessage);
            }
          });
      }
    },
  },

  getters: {
    runningWorkerStatus(state: any): RunningWorkerStatusDTO {
      return state.runningWorkerStatus;
    },
    runningWorkerStatusFiles(state: any): string[] {
      return state.runningWorkerStatusFiles;
    },
  },
};
