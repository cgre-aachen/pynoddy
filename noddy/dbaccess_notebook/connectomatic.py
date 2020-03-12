def connectomatic():
    connection = psycopg2.connect(user = "postgres",
                                  password = "loop123pgpw",
                                  host = "130.95.198.59",
                                  port = "5432",
                                  database = "noddyverse")
    return connection

def convertTuple(tup): 
    val = functools.reduce(operator.add, (tup)) 
    return val

def getSome(field, table, condition_field,condition_type,condition_value,many):
    c=connection.cursor()
    if(condition_type==0 and condition_field !=''):
        postgreSQL_select_Query = "SELECT "+field+" FROM "+table+" WHERE "+condition_field+" LIKE "
        postgreSQL_select_Query=postgreSQL_select_Query.replace('"','').replace("'",'')
        postgreSQL_select_Query=postgreSQL_select_Query+"'"+condition_value+"'"
    elif(condition_type==1 and condition_field !=''):
        postgreSQL_select_Query = "SELECT "+field+"' FROM "+table+" WHERE "+condition_field+" = "
        postgreSQL_select_Query=postgreSQL_select_Query.replace('"','').replace("'",'')
        postgreSQL_select_Query=postgreSQL_select_Query+"'"+condition_value+"'"
    else:
        postgreSQL_select_Query = "SELECT '"+field+"' FROM '"+table+"'"
        postgreSQL_select_Query=postgreSQL_select_Query.replace('"','').replace("'",'')

    #print(postgreSQL_select_Query)    
    c.execute(postgreSQL_select_Query)
    if(many==1):
        result = c.fetchall() 
    else:
        result = c.fetchone() 
    
    result = convertTuple(result)
    return result