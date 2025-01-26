const DEFAULT_OPTIONS = {
  cache: 'no-store',
  credentials: 'same-origin',
  headers: {
    'Content-Type': 'application/json'
    // 'Content-Type': 'application/x-www-form-urlencoded',
  }
};

const serverFetch = async (url, init = {}) => {
  const options = { ...DEFAULT_OPTIONS, ...init };
  const response = await fetch(url, options);
  return response;
};

const serverPost = (url, body) => {
  return serverFetch(url, { method: 'POST', body });
};

export const serverGet = (url) => {
  return serverFetch(url, { method: 'GET' });
};

export const getCgiResponse = async (path) => {
  const resp = (await fetch(path)).text();
  return resp;
};

export const getCgiResponseJSON = async (path) => {
  const resp = (await fetch(path)).json();
  return resp;
};

export const jsonRequest = async (url, body) => {
  try {
    const response = await serverPost(url, JSON.stringify(body));
    return response.json();
  } catch (error) {
    // Response probably not JSON
    console.error(error);
  }
};

export const cgiRequest = async (url, body) => {
  try {
    const response = await serverPost(url, JSON.stringify(body));
    return response;
  } catch (error) {
    console.error(error);
  }
};

export const getParam = async (param) => {
  const PARAMS_BASE_PATH = '/axis-cgi/param.cgi?action=list&group=';
  try {
    const resp = await getCgiResponse(`${PARAMS_BASE_PATH}${param}`);
    const parsedData = resp.substring(resp.indexOf('=') + 1);
    return parsedData;
  } catch (error) {
    console.error(error);
    return error;
  }
};
