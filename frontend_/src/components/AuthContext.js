import { createContext, useContext, useState, useEffect } from "react";

const AuthContext = createContext(null);

export const AuthProvider = ({ children }) => {
  const [user, setUser] = useState(null);

  useEffect(() => {
    const token = localStorage.getItem("access_token");
    const username = localStorage.getItem("username");
    const user_id = localStorage.getItem("user_id");

    if (token && username && user_id) {
      setUser({ token, username, user_id });
    }
  }, []);

  const login = (token, username, user_id) => {
    localStorage.setItem("access_token", token);
    localStorage.setItem("username", username);
    localStorage.setItem("user_id", user_id);
    setUser({ token, username, user_id });
  };

  const logout = () => {
    localStorage.removeItem("access_token");
    localStorage.removeItem("username");
    localStorage.removeItem("user_id");
    setUser(null);
  };

  return (
    <AuthContext.Provider value={{ user, login, logout }}>
      {children}
    </AuthContext.Provider>
  );
};

export const useAuth = () => useContext(AuthContext);
