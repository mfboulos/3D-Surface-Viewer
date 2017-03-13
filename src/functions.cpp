GLfloat mult(GLfloat a, GLfloat b)
{
   return (a * b);
}

GLfloat div(GLfloat a, GLfloat b)
{
   return (a / b);
}

GLfloat plus(GLfloat a, GLfloat b)
{
   return (a + b);
}

GLfloat minus(Glfloat a, GLfloat b)
{
   return (a - b);
}

GLfloat s(GLfloat a)
{
   return sin(a);
}

GLfloat c(GLfloat a)
{
   return cos(a);
}

GLfloat t(GLfloat a)
{
   return tan(a);
}

GLfloat power(GLfloat a, GLfloat b)
{
   return pow(a, b);
}

GLfloat natLog(GLfloat a)
{
   return log(a);
}

GLfloat evalExpression(String func, GLfloat x, GLfloat y, int start, int end)
{
/*   int parentheses = 0;
   for(int i = start; i <= end; i++)
   {
      if(func[i] == '+')
         return (evalExpression(func, x, y, start, i - 1) + evalExpression(func, x, y, i + 1, end));
      else if(func[i] == '-')
         return (evalExpression(func, x, y, start, i - 1) - evalExpression(func, x, y, i + 1, end));
      else if(func[i] == '(')
      {
         parentheses = 1;
         for(int j = i; j <= end && parentheses != 0; j++)
         {
            if(func[j] == '(')
               parentheses++;
            else if(func[j] == ')')
               parentheses--;
         }
         return */

