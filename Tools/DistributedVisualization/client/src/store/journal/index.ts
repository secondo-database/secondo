import { JournalDTO } from "@/dto/Journal";
import axios, { AxiosResponse } from "axios";
import config from "../../config/default";
import {
  SET_JOURNAL_ITEMS,
  SET_GLOBAL_LOADING,
  SET_GLOBAL_ERROR,
  CLEAR_GLOBAL_ERROR,
  SET_WORKERSTATUS_FILES,
  SET_TREE_FOR_DISPLAY,
  SET_SIMPLIFIED_ARGS,
} from "../mutation_types";
import { TreeForDisplayDTO } from "@/dto/TreeForDisplay";
import { TreeService } from "@/service/Tree";
import { SimplifiedArgsDTO } from "@/dto/SimplifiedArgsDTO";

const apiUrl: string = config.base_url + config.api_version + config.journal_path;

export default {
  state: {
    journalItems: ([] as JournalDTO[]),
    simplifiedArgs: ({} as SimplifiedArgsDTO),
    treeForDisplay: ({} as TreeForDisplayDTO),
  },
  mutations: {
    [SET_JOURNAL_ITEMS](state: any, journalItems: JournalDTO[]): void {
      if (journalItems.length > 0) {
        state.journalItems = journalItems;
        TreeService.prepareTreeForDisplay(state.journalItems[0]);
      }
    },
    [SET_TREE_FOR_DISPLAY](state: any, treeForDisplay: TreeForDisplayDTO): void {
      state.treeForDisplay = treeForDisplay;
    },
    [SET_SIMPLIFIED_ARGS](state: any, simplifiedArgs: SimplifiedArgsDTO): void {
      state.simplifiedArgs = simplifiedArgs;
    },
  },

  actions: {
    getJournalAction({ commit, dispatch }: any): void {
      axios.get(apiUrl, {
        headers: {
          // no headers
        },
      }).
        then((getJournalResponse: AxiosResponse) => {
          commit(SET_JOURNAL_ITEMS, getJournalResponse.data);
          commit(SET_WORKERSTATUS_FILES);
          commit(SET_GLOBAL_LOADING, false);
          commit(CLEAR_GLOBAL_ERROR);
        }).catch((getJournalError: any) => {
          if (getJournalError.response !== undefined) {
            const errMessage = getJournalError.response.data.message;
            commit(SET_GLOBAL_LOADING, false);
            commit(SET_GLOBAL_ERROR, errMessage);
          } else {
            const errMessage = "Server not available";
            commit(SET_GLOBAL_LOADING, false);
            commit(SET_GLOBAL_ERROR, errMessage);
          }

        });
    },
  },
  getters: {
    journalItems(state: any): JournalDTO[] {
      return state.journalItems;
    },
    simplifiedArgs(state: any): SimplifiedArgsDTO {
      return state.simplifiedArgs;
    },
    treeForDisplay(state: any): TreeForDisplayDTO {
      return state.treeForDisplay;
    },
  },
};
